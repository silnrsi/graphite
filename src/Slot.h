#ifndef SLOT_INCLUDE
#define SLOT_INCLUDE

#include "graphiteng/ISegment.h"
#include "graphiteng/ISlot.h"
#include "graphiteng/Types.h"
#include "FontImpl.h"

#define SLOT_DELETED    1
#define SLOT_INSERT	2

class Segment;


class Slot : public ISlot
{
public:
    virtual unsigned short gid() const { return m_glyphid; }
    virtual Position origin() const { return m_position; }
    virtual float advance(FontImpl *font) { return m_advance.x < 0 ? font->advance(m_glyphid) : font->scale(m_advance.x); }
    virtual int before() const { return m_before; }
    virtual int after() const { return m_after; }

    Slot();
    void glyph(unsigned short glyphid) { m_glyphid = glyphid; }
    void origin(Position &pos) { m_position = pos + m_shift; }
    void originate(int index) { m_original = index; }
    int original() { return m_original; }
    void before(int index) { m_before = index; }
    void after(int index) { m_after = index; }
    bool isBase() { return (m_parent == -1); }
    void update(int numSlots, int numCharInfo, Position &relpos);
    void finalise(Segment *seg, FontImpl *font, Position &base, float *cMin, float *cMax);
    bool isDeleted() { return (m_flags & SLOT_DELETED) ? true : false; }
    void markDeleted(bool state) { if (state) m_flags |= SLOT_DELETED; else m_flags &= ~SLOT_DELETED; }
    bool isInsertBefore() { return (m_flags & SLOT_INSERT) ? true : false; }
    void markInsertBefore(bool state) { if (state) m_flags |= SLOT_INSERT; else m_flags &= ~SLOT_INSERT; }
    int getAttr(Segment *seg, uint8 index, uint8 subindex);
    void setAttr(Segment *seg, uint8 index, uint8 subindex, int value);
    void attachTo(int ap) { m_attachto = ap; }
    int attachTo() { return m_attachto; }

    enum attrCode {
	kslatAdvX = 0, kslatAdvY,
	kslatAttTo,
	kslatAttX, kslatAttY,
	kslatAttXOff, kslatAttYOff,
	kslatAttWithX, kslatAttWithY,
	kslatAttWithXOff, kslatAttWithYOff,
	kslatAttLevel,
	kslatBreak,
	kslatCompRef,
	kslatDir,
	kslatInsert,
	kslatPosX, kslatPosY,
	kslatShiftX, kslatShiftY,
	kslatUserDefnV1,
	kslatMeasureSol, kslatMeasureEol,
	kslatJStretch, kslatJShrink, kslatJStep, kslatJWeight, kslatJWidth,
	
	kslatUserDefn = kslatJStretch + 30,
	
	kslatMax,
	kslatNoEffect = kslatMax + 1
    };
    
protected:
    unsigned short m_glyphid;        // glyph id
    int m_original;	    // charinfo that originated this slot (e.g. for feature values)
    int m_before;           // charinfo index of before association
    int m_after;            // charinfo index of after association
    int m_parent;           // index to parent we are attached to
    int m_child;            // index to first child slot that attaches to us
    int m_sibling;          // index to next child that attaches to our parent
    int m_attachto;        // temporary action code, attach.to
    Position m_position;    // absolute position of glyph
    Position m_shift;       // .shift slot attribute
    Position m_advance;     // .advance slot attribute
    Position m_attach;      // attachment point on us
    Position m_with;	    // attachment point position on parent
    byte m_flags;           // holds bit flags
};

#endif // SLOT_INCLUDE

