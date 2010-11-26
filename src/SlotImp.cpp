/*  GRAPHITENG LICENSING

    Copyright 2010, SIL International
    All rights reserved.

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published
    by the Free Software Foundation; either version 2.1 of License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should also have received a copy of the GNU Lesser General Public
    License along with this library in the file named "LICENSE".
    If not, write to the Free Software Foundation, Inc., 59 Temple Place, 
    Suite 330, Boston, MA 02111-1307, USA or visit their web page on the 
    internet at http://www.fsf.org/licenses/lgpl.html.
*/
#include "GrSegmentImp.h"
#include "SlotImp.h"
#include "CharInfoImp.h"
#include "Rule.h"

using namespace org::sil::graphite::v2;

Slot::Slot() :
    m_next(NULL), m_prev(NULL),
    m_glyphid(0), m_realglyphid(0), m_original(0), m_before(0), m_after(0),
    m_parent(NULL), m_child(NULL), m_sibling(NULL),
    m_position(0, 0), m_shift(0, 0), m_advance(-1, -1),
    m_attach(0, 0), m_with(0, 0),
    m_flags(0)
    // Do not set m_userAttr since it is set *before* new is called since this
    // is used as a positional new to reset the Slot
{
}

// take care, this does not copy any of the Slot pointer fields
void Slot::set(const Slot & orig, int charOffset, uint8 numUserAttr)
{
    // leave m_next and m_prev unchanged
    m_glyphid = orig.m_glyphid;
    m_realglyphid = orig.m_realglyphid;
    m_original = orig.m_original + charOffset;
    m_before = orig.m_before + charOffset;
    m_after = orig.m_after + charOffset;
    m_parent = NULL;
    m_child = NULL;
    m_sibling = NULL;
    m_position = orig.m_position;
    m_shift = orig.m_shift;
    m_advance = orig.m_advance;
    m_attach = orig.m_attach;
    m_with = orig.m_with;
    m_flags = orig.m_flags;
    m_attLevel = orig.m_attLevel;
    assert(!orig.m_userAttr || m_userAttr);
    if (m_userAttr && orig.m_userAttr)
    {
        memcpy(m_userAttr, orig.m_userAttr, numUserAttr * sizeof(*m_userAttr));
    }
}

void Slot::update(int numSlots, int numCharInfo, Position &relpos)
{
    m_before += numCharInfo;
    m_after += numCharInfo;
    m_position = m_position + relpos;
};

Position Slot::finalise(const GrSegment *seg, const GrFont *font, Position *base, Rect *bbox, float *cMin, uint8 attrLevel)
{
    if (attrLevel && m_attLevel > attrLevel) return Position(0, 0);
    float scale = 1.0;
    Position shift = m_shift;
    float tAdvance = m_advance.x;
    const GlyphFace * glyphFace = seg->getFace()->getGlyphFaceCache()->glyphSafe(glyph());
    if (font)
    {
        scale = font->scale();
        shift *= scale;
        if (font->isHinted())
        {
            if (glyphFace)
                tAdvance = (m_advance.x - glyphFace->theAdvance().x) * scale + font->advance(m_glyphid);
            else
                tAdvance = (m_advance.x - seg->glyphAdvance(glyph())) * scale + font->advance(m_glyphid);
        }
        else
            tAdvance *= scale;
    }    
//    float scale = font ? font->scale() : 1.0;
//    Position shift = m_shift * scale;
//    float tAdvance = font ? (m_advance.x - seg->glyphAdvance(glyph())) * scale + font->advance(m_glyphid) : m_advance.x;
    Position res;

    m_position = *base + shift;
    if (!m_parent)
    {
        res = *base + Position(tAdvance, m_advance.y);
        *cMin = 0.;
    }
    else
    {
        float tAdv;
        m_position += (m_attach - m_with) * scale;
        tAdv = tAdvance > 0. ? m_position.x + tAdvance - shift.x : 0.;
        res = Position(tAdv, 0);
    }

    if (glyphFace)
    {
        Rect ourBbox = glyphFace->theBBox() * scale + m_position;
        bbox->widen(ourBbox);
    }
    //Rect ourBbox = seg->theGlyphBBoxTemporary(glyph()) * scale + m_position;
    //bbox->widen(ourBbox);

    if (m_parent && m_position.x < *cMin) *cMin = m_position.x;

    if (m_child)
    {
        Position tRes = m_child->finalise(seg, font, &m_position, bbox, cMin, attrLevel);
        if (tRes.x > res.x) res = tRes;
    }

    if (m_sibling)
    {
        Position tRes = m_sibling->finalise(seg, font, base, bbox, cMin, attrLevel);
        if (tRes.x > res.x) res = tRes;
    }
    
    if (!m_parent && *cMin < 0)
    {
        Position adj = Position(-*cMin, 0.);
        res += adj;
        m_position += adj;
        if (m_child) m_child->floodShift(adj);
    }
    return res;
}

uint32 Slot::clusterMetric(const GrSegment *seg, uint8 metric, uint8 attrLevel)
{
    Position base;
    Rect bbox;
    float cMin = 0.;
    Position res = finalise(seg, NULL, &base, &bbox, &cMin, attrLevel);

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

int Slot::getAttr(const GrSegment *seg, attrCode index, uint8 subindex) const
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
        return 0;
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
        return m_position.x; // but need to calculate it
    case kslatPosY :
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
        return m_userAttr[subindex];
    default :
        return 0;
    }
}

void Slot::setAttr(GrSegment *seg, attrCode index, uint8 subindex, uint16 val, const SlotMap & map)
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
        if (value >= 0 && unsigned(value) < map.size())
        {
            Slot *other = map[value];
            m_parent = other;
            other->child(this);
            m_attach = Position(seg->glyphAdvance(other->gid()), 0);
        }
        else
        {
#ifndef DISABLE_TRACING
            XmlTraceLog::get().warning("invalid slatAttTo %d", value);
#endif
        }
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
        m_userAttr[subindex] = value;
        break;
    default :
        break;
    }
}

void Slot::child(Slot *ap)
{
    if (ap == m_child) {}
    else if (!m_child)
        m_child = ap;
    else
        m_child->sibling(ap);
}

void Slot::sibling(Slot *ap)
{
    if (ap == m_sibling) {}
    else if (!m_sibling)
        m_sibling = ap;
    else
        m_sibling->sibling(ap);
}

void Slot::setGlyph(GrSegment *seg, uint16 glyphid, const GlyphFace * theGlyph)
{
    m_glyphid = glyphid;
    if (!theGlyph)
    {
        theGlyph = seg->getFace()->getGlyphFaceCache()->glyphSafe(glyphid);
        if (!theGlyph)
        {
            m_realglyphid = 0;
            m_advance = Position(0.,0.);
            return;
        }
    }
    m_realglyphid = theGlyph->getAttr(seg->silf()->aPseudo());
    m_advance = Position(theGlyph->theAdvance().x, 0.);
}

void Slot::floodShift(Position adj)
{
    m_position += adj;
    if (m_child) m_child->floodShift(adj);
    if (m_sibling) m_sibling->floodShift(adj);
}