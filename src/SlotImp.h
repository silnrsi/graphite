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
#pragma once

#include "graphite2/Types.h"
#include "graphite2/Segment.h"
#include "Main.h"
#include "GrFontImp.h"

namespace org { namespace sil { namespace graphite { namespace v2 {

#define SLOT_DELETED    1
#define SLOT_INSERT	2
#define SLOT_COPIED     4
#define SLOT_POSITIONED 8

typedef gr_attrCode attrCode;

struct GrSegment;

struct GrSlot
{
public:
    unsigned short gid() const { return m_glyphid; }
    Position origin() const { return m_position; }
    float advance() const { return m_advance.x; }
    int before() const { return m_before; }
    int after() const { return m_after; }

    GrSlot();
    void set(const GrSlot & slot, int charOffset, uint8 numUserAttr);
    GrSlot *next() const { return m_next; }
    void next(GrSlot *s) { m_next = s; }
    GrSlot *prev() const { return m_prev; }
    void prev(GrSlot *s) { m_prev = s; }
    uint16 glyph() const { return m_realglyphid ? m_realglyphid : m_glyphid; }
    void setGlyph(GrSegment *seg, uint16 glyphid, const GlyphFace * theGlyph = NULL);
    void setRealGid(uint16 realGid) { m_realglyphid = realGid; }
    void origin(const Position &pos) { m_position = pos + m_shift; }
    void originate(int index) { m_original = index; }
    int original() const { return m_original; }
    void before(int index) { m_before = index; }
    void after(int index) { m_after = index; }
    bool isBase() const { return (!m_parent); }
    void update(int numSlots, int numCharInfo, Position &relpos);
    Position finalise(const GrSegment* seg, const GrFont* font, Position* base, Rect* bbox, float* cMin, uint8 attrLevel);
    bool isDeleted() const { return (m_flags & SLOT_DELETED) ? true : false; }
    void markDeleted(bool state) { if (state) m_flags |= SLOT_DELETED; else m_flags &= ~SLOT_DELETED; }
    bool isCopied() const { return (m_flags & SLOT_COPIED) ? true : false; }
    void markCopied(bool state) { if (state) m_flags |= SLOT_COPIED; else m_flags &= ~SLOT_COPIED; }
    bool isPositioned() const { return (m_flags & SLOT_POSITIONED) ? true : false; }
    void markPositioned(bool state) { if (state) m_flags |= SLOT_POSITIONED; else m_flags &= ~SLOT_POSITIONED; }
    bool isInsertBefore() const { return (m_flags & SLOT_INSERT) ? true : false; }
    uint16 *userAttrs() { return m_userAttr; }
    void userAttrs(uint16 *p) { m_userAttr = p; }
    void markInsertBefore(bool state) { if (state) m_flags |= SLOT_INSERT; else m_flags &= ~SLOT_INSERT; }
    void setAttr(GrSegment* seg, attrCode index, uint8 subindex, int16 val, const SlotMap & map);
    int getAttr(const GrSegment *seg, attrCode index, uint8 subindex) const;
    void attachTo(GrSlot *ap) { m_parent = ap; }
    GrSlot *attachedTo() const { return m_parent; }
    GrSlot* firstChild() const { return m_child; }
    void child(GrSlot *ap);
    GrSlot* nextSibling() const { return m_sibling; }
    void sibling(GrSlot *ap);
    GrSlot *attachTo() const { return m_parent; }
    uint32 clusterMetric(const GrSegment* seg, uint8 metric, uint8 attrLevel);
    void positionShift(Position a) { m_position += a; }
    void floodShift(Position adj);

    CLASS_NEW_DELETE

private:
    GrSlot *m_next;           // linked list of slots
    GrSlot *m_prev;
    unsigned short m_glyphid;        // glyph id
    uint16 m_realglyphid;
    uint32 m_original;	    // charinfo that originated this slot (e.g. for feature values)
    uint32 m_before;           // charinfo index of before association
    uint32 m_after;            // charinfo index of after association
    GrSlot *m_parent;         // index to parent we are attached to
    GrSlot *m_child;          // index to first child slot that attaches to us
    GrSlot *m_sibling;        // index to next child that attaches to our parent
    Position m_position;    // absolute position of glyph
    Position m_shift;       // .shift slot attribute
    Position m_advance;     // .advance slot attribute
    Position m_attach;      // attachment point on us
    Position m_with;	    // attachment point position on parent
    byte m_flags;           // holds bit flags
    byte m_attLevel;        // attachment level
    uint16 *m_userAttr;     // pointer to user attributes
};

typedef struct GrSlot Slot;

}}}} // namespace
