#include "Segment.h"
#include "Slot.h"
#include "CharInfo.h"

Slot::Slot() :
        m_glyphid(0), m_realglyphid(0), m_before(0), m_after(0), m_parent(-1), m_child(-1), m_sibling(-1),
        m_position(0, 0), m_advance(-1, -1), m_shift(0, 0)
{
}

void Slot::update(int numSlots, int numCharInfo, Position &relpos)
{
    m_before += numCharInfo;
    m_after += numCharInfo;
    m_position = m_position + relpos;
};

Position Slot::finalise(Segment *seg, const LoadedFont *font, Position *base, Rect *bbox, float *cMin, uint8 attrLevel)
{
    if (attrLevel && m_attLevel > attrLevel) return Position(0, 0);
    float scale = font ? font->scale() : 1.0;
    Position shift = m_shift * scale;
    float tAdvance = font ? (m_advance.x - seg->glyphAdvance(glyph())) * scale + advance(font) : m_advance.x;
//    float tAdvance = font ? m_advance.x * scale + advance(font) : m_advance.x + seg->glyphAdvance(m_glyphid);
    Position res;

    m_position = *base + shift;
    if (m_parent == -1)
        res = *base + Position(tAdvance, m_advance.y);
    else
    {
        float tAdv;
        m_position += (m_attach - m_with) * scale;
        tAdv = m_position.x + tAdvance - shift.x;
        res = Position(tAdv, 0);
    }

    Rect ourBbox = seg->glyphBbox(glyph()) * scale + m_position;
    bbox->widen(ourBbox);

    if (m_parent != -1 && ourBbox.bl.x < *cMin) *cMin = ourBbox.bl.x;

    if (m_child != -1)
    {
        Position tRes = (*seg)[m_child].finalise(seg, font, &m_position, bbox, cMin, attrLevel);
        if (tRes.x > res.x) res = tRes;
    }

    if (m_sibling != -1)
    {
        Position tRes = (*seg)[m_sibling].finalise(seg, font, base, bbox, cMin, attrLevel);
        if (tRes.x > res.x) res = tRes;
    }
    return res;
}

uint32 Slot::clusterMetric(const Segment *seg, int is, uint8 metric, uint8 attrLevel) const
{
    Position base;
    Rect bbox;
    float cMin = 0.;
    Position res = const_cast<Segment *>(seg)->finalise(is, NULL, &base, &bbox, &cMin, attrLevel);

    switch ((enum metrics)metric)
    {
    case kgmetLsb :
        return bbox.bl.x;
    case kgmetRsb :
        return res.x - bbox.tr.x;
    case kgmetBbTop :
        return bbox.tr.y;
    case kgmetBbBottom :
        return bbox.bl.y;
    case kgmetBbLeft :
        return bbox.bl.x;
    case kgmetBbRight :
        return bbox.tr.x;
    case kgmetBbWidth :
        return bbox.tr.x - bbox.bl.x;
    case kgmetBbHeight :
        return bbox.tr.y - bbox.bl.y;
    case kgmetAdvWidth :
        return res.x;
    case kgmetAdvHeight :
        return res.y;
    default :
        return 0;
    }
}

int Slot::getAttr(const Segment *seg, attrCode index, uint8 subindex, int is, int *startSlot, int *endSlot, Position *endPos) const
{
    if (index == kslatUserDefnV1)
    {
        index = kslatUserDefn;
        subindex = 0;
    }
    switch (index)
    {
    case kslatAdvX :
        return m_advance.x;
    case kslatAdvY :
        return m_advance.y;
    case kslatAttTo :
        return m_parent;
    case kslatAttX :
        return m_attach.x;
    case kslatAttY :
        return m_attach.y;
    case kslatAttXOff :
        return 0;
    case kslatAttYOff :
        return 0;
    case kslatAttWithX :
        return m_with.x;
    case kslatAttWithY :
        return m_with.y;
    case kslatAttWithXOff :
        return 0;
    case kslatAttWithYOff :
        return 0;
    case kslatAttLevel :
        return m_attLevel;
    case kslatBreak :
        return seg->charinfo(m_original)->breakWeight();
    case kslatCompRef :
        return 0;
    case kslatDir :
        return seg->dir();
    case kslatInsert :
        return isInsertBefore();
    case kslatPosX :
        const_cast<Segment *>(seg)->positionSlots(is, startSlot, endSlot, endPos);
        return m_position.x; // but need to calculate it
    case kslatPosY :
        const_cast<Segment *>(seg)->positionSlots(is, startSlot, endSlot, endPos);
        return m_position.y;
    case kslatShiftX :
        return m_shift.x;
    case kslatShiftY :
        return m_shift.y;
    case kslatMeasureSol :
        return -1; // err what's this?
    case kslatMeasureEol :
        return -1;
    case kslatJStretch :
        return 0;
    case kslatJShrink :
        return 0;
    case kslatJStep :
        return 0;
    case kslatJWeight :
        return 0;
    case kslatJWidth :
        return 0;
    case kslatUserDefn :
        return const_cast<Segment *>(seg)->user(is, subindex);
    default :
        return 0;
    }
}

void Slot::setAttr(Segment *seg, attrCode index, uint8 subindex, uint16 val, int is)
{
    int value = *(int16 *)&val;
    if (index == kslatUserDefnV1)
    {
        index = kslatUserDefn;
        subindex = 0;
    }
    switch (index)
    {
    case kslatAdvX :
        m_advance = Position(value, m_advance.y);
        break;
    case kslatAdvY :
        m_advance = Position(m_advance.x, value);
        break;
    case kslatAttTo :
        m_parent = value;
        (*seg)[value].child(seg, is);
        break;
    case kslatAttX :
        m_attach = Position(value, m_attach.y);
        break;
    case kslatAttY :
        m_attach = Position(m_attach.x, value);
        break;
    case kslatAttXOff :
        break;
    case kslatAttYOff :
        break;
    case kslatAttWithX :
        m_with = Position(value, m_with.y);
        break;
    case kslatAttWithY :
        m_with = Position(m_with.x, value);
        break;
    case kslatAttWithXOff :
        break;
    case kslatAttWithYOff :
        break;
    case kslatAttLevel :
        m_attLevel = value;
        break;
    case kslatBreak :
        seg->charinfo(m_original)->breakWeight(value);
        break;
    case kslatCompRef :
        break;      // not sure what to do here
    case kslatDir :
        break;  // read only
    case kslatInsert :
        markInsertBefore(value? true : false);
        break;
    case kslatPosX :
        break; // can't set these here
    case kslatPosY :
        break;
    case kslatShiftX :
        m_shift = Position(value, m_shift.y);
        break;
    case kslatShiftY :
        m_shift = Position(m_shift.x, value);
        break;
    case kslatMeasureSol :
        break;
    case kslatMeasureEol :
        break;
    case kslatJStretch :
        break;  // handle these later
    case kslatJShrink :
        break;
    case kslatJStep :
        break;
    case kslatJWeight :
        break;
    case kslatJWidth :
        break;
    case kslatUserDefn :
        seg->user(is, subindex, value);
        break;
    default :
        break;
    }
}

void Slot::child(Segment *seg, int ap)
{
    if (ap == m_child) {}
    else if (ap == -1 || m_child == -1)
        m_child = ap;
    else
        (*seg)[m_child].child(seg, ap);
}

void Slot::setGlyph(Segment *seg, uint16 glyphid)
{
    m_glyphid = glyphid;
    m_realglyphid = seg->glyphAttr(glyphid, seg->silf()->aPseudo());
    m_advance = Position(seg->glyphAdvance(glyphid), 0.);
}
