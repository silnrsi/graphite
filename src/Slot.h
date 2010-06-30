#pragma once

#include "graphiteng/Types.h"
#include "graphiteng/SegmentHandle.h"
#include "LoadedFont.h"

#define SLOT_DELETED    1
#define SLOT_INSERT	2

class Segment;

class Slot
{
public:
    unsigned short gid() const { return m_glyphid; }
    Position origin() const { return m_position; }
    float advance(const LoadedFont *font) const { return m_advance.x < 0 ? font->advance(m_glyphid) : (font ? font->scale(m_advance.x) : m_advance.x); }
    int before() const { return m_before; }
    int after() const { return m_after; }

    Slot();
    void glyph(unsigned short glyphid) { m_glyphid = glyphid; }
    void setGlyph(Segment *seg, uint16 glyphid);
    void origin(const Position &pos) { m_position = pos + m_shift; }
    void originate(int index) { m_original = index; }
    int original() const { return m_original; }
    void before(int index) { m_before = index; }
    void after(int index) { m_after = index; }
    bool isBase() const { return (m_parent == -1); }
    void update(int numSlots, int numCharInfo, Position &relpos);
    Position finalise(Segment* seg, const LoadedFont* font, Position* base, Rect* bbox, float* cMin, uint8 attrLevel);
    bool isDeleted() const { return (m_flags & SLOT_DELETED) ? true : false; }
    void markDeleted(bool state) { if (state) m_flags |= SLOT_DELETED; else m_flags &= ~SLOT_DELETED; }
    bool isInsertBefore() const { return (m_flags & SLOT_INSERT) ? true : false; }
    void markInsertBefore(bool state) { if (state) m_flags |= SLOT_INSERT; else m_flags &= ~SLOT_INSERT; }
    void setAttr(Segment *seg, attrCode index, uint8 subindex, uint16 value, int is);
    int getAttr(const Segment* seg, attrCode index, uint8 subindex, int is) const;
    void attachTo(int ap) { m_parent = ap; }
    void child(Segment *seg, int ap);
    int attachTo() const { return m_parent; }
    uint32 clusterMetric(const Segment *seg, int is, uint8 metric, uint8 attrLevel) const;

private:
    unsigned short m_glyphid;        // glyph id
    int m_original;	    // charinfo that originated this slot (e.g. for feature values)
    int m_before;           // charinfo index of before association
    int m_after;            // charinfo index of after association
    int m_parent;           // index to parent we are attached to
    int m_child;            // index to first child slot that attaches to us
    int m_sibling;          // index to next child that attaches to our parent
    Position m_position;    // absolute position of glyph
    Position m_shift;       // .shift slot attribute
    Position m_advance;     // .advance slot attribute
    Position m_attach;      // attachment point on us
    Position m_with;	    // attachment point position on parent
    byte m_flags;           // holds bit flags
    byte m_attLevel;        // attachment level
};


