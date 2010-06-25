#include "Slot.h"
#include "CharInfo.h"
#include "Segment.h"

Slot::Slot() :
    m_glyphid(0), m_before(0), m_after(0), m_parent(-1), m_child(-1), m_sibling(-1),
    m_position(0, 0), m_advance(-1, -1), m_shift(0, 0)
{
}

void Slot::update(int numSlots, int numCharInfo, Position &relpos)
{
    m_before += numCharInfo;
    m_after += numCharInfo;
    m_position = m_position + relpos;
};

void Slot::finalise(Segment *seg, FontImpl *font, Position &base, float *cMin, float *cMax)
{
    Position shift = font->scale(m_shift);
    m_position = base + shift;
    if (m_parent == -1)
        *cMax = m_position.x + advance(font) - shift.x;
    else
    {
        float tAdv;
        m_position += font->scale(m_attach);
        tAdv = m_position.x + (m_advance.x > 0 ? font->scale(m_advance.x) : advance(font)) - shift.x;
        if (tAdv > *cMax) *cMax = tAdv;
        if (m_position.x - base.x - shift.x < *cMin) *cMin = m_position.x - base.x - shift.x;
    }

    if (m_child != -1)
        (*seg)[m_child].finalise(seg, font, m_position, cMin, cMax);

    if (m_sibling != -1)
        (*seg)[m_sibling].finalise(seg, font, base, cMin, cMax);
}

int Slot::getAttr(Segment *seg, uint8 index, uint8 subindex)
{
    if ((enum attrCode)index == kslatUserDefnV1)
    {
	index = kslatUserDefn;
	subindex = 0;
    }
    switch ((enum attrCode)index)
    {
	case kslatAdvX : return m_advance.x;
	case kslatAdvY : return m_advance.y;
	case kslatAttTo : return m_parent;
	case kslatAttX : return m_attach.x;
	case kslatAttY : return m_attach.y;
	case kslatAttXOff : return 0;
	case kslatAttYOff : return 0;
	case kslatAttWithX : return m_with.x;
	case kslatAttWithY : return m_with.y;
	case kslatAttWithXOff : return 0;
	case kslatAttWithYOff : return 0;
	case kslatBreak : seg->charinfo(m_original)->breakWeight();
	case kslatCompRef : return 0;
	case kslatDir : return seg->dir();
	case kslatInsert : return isInsertBefore();
	case kslatPosX : return m_position.x; // but need to calculate it
	case kslatPosY : return m_position.y;
	case kslatShiftX : return m_shift.x;
	case kslatShiftY : return m_shift.y;
	case kslatMeasureSol : return -1; // err what's this?
	case kslatMeasureEol : return -1;
	case kslatJStretch : return 0;
	case kslatJShrink : return 0;
	case kslatJStep : return 0;
	case kslatJWeight : return 0;
	case kslatJWidth : return 0;
	case kslatUserDefn : return 0; // get it from the seg
	default : return 0;
    }
}

void Slot::setAttr(Segment *seg, uint8 index, uint8 subindex, int value)
{
    if ((enum attrCode)index == kslatUserDefnV1)
    {
	index = kslatUserDefn;
	subindex = 0;
    }
    switch ((enum attrCode)index)
    {
	case kslatAdvX : m_advance = Position(value, m_advance.y); break;
	case kslatAdvY : m_advance = Position(m_advance.x, value); break;
	case kslatAttTo : m_parent = value; break;
	case kslatAttX : m_attach = Position(value, m_attach.y); break;
	case kslatAttY : m_attach = Position(m_attach.x, value); break;
	case kslatAttXOff : break;
	case kslatAttYOff : break;
	case kslatAttWithX : m_with = Position(value, m_with.y); break;
	case kslatAttWithY : m_with = Position(m_with.x, value); break;
	case kslatAttWithXOff : break;
	case kslatAttWithYOff : break;
	case kslatBreak : seg->charinfo(m_original)->breakWeight(value); break;
	case kslatCompRef : break;		// not sure what to do here
	case kslatDir : break;	// read only
	case kslatInsert : markInsertBefore(value? true : false); break;
	case kslatPosX : break; // can't set these here
	case kslatPosY : break;
	case kslatShiftX : m_shift = Position(value, m_shift.y); break;
	case kslatShiftY : m_shift = Position(m_shift.x, value); break;
	case kslatMeasureSol : break;
	case kslatMeasureEol : break;
	case kslatJStretch : break;	// handle these later
	case kslatJShrink : break;
	case kslatJStep : break;
	case kslatJWeight : break;
	case kslatJWidth : break;
	case kslatUserDefn : break;	// talk to the seg
	default : break;
    }
}
