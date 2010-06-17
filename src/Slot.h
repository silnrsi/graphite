#ifndef SLOT_INCLUDE
#define SLOT_INCLUDE

#include "graphiteng/ISlot.h"
#include "graphiteng/Types.h"
#include "FontImpl.h"

class Slot : public ISlot
{
public:
    virtual unsigned short gid() { return m_glyphid; }
    virtual int next() { return m_next; }
    virtual Position origin() { return m_position; }
    virtual float advance(FontImpl *font) { return font->advance(m_glyphid) + m_kern.x; }
    virtual int before() { return m_before; }
    virtual int after() { return m_after; }

    Slot();
    void init(int id) { m_id = id; }
    void glyph(unsigned short glyphid) { m_glyphid = glyphid; }
    void next(int index) { m_next = index; }
    void prev(int index) { m_prev = index; }
    void origin(Position &pos) { m_position = pos + m_shift + m_kern; }
    void before(int index) { m_before = index; }
    void after(int index) { m_after = index; }
    void update(int numSlots, int numCharInfo, Position &relpos);

protected:
    int m_id;               // index in the parent segment slots array
    int m_next;             // link to next slot in stream
    int m_prev;             // link to previous slot in stream
    unsigned short m_glyphid;        // glyph id
    int m_before;           // charinfo index of before association
    int m_after;            // charinfo index of after association
    Position m_position;    // absolute position of glyph
    Position m_shift;       // .shift slot attribute
    Position m_kern;        // .kern slot attribute
    Position m_attach;      // shift relative to parent due to attachment
};

#endif // SLOT_INCLUDE

