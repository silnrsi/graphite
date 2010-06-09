#ifndef SLOT_INCLUDE
#define SLOT_INCLUDE

#include "Font.h"
#include "Misc.h"

class Slot // : ISlot
{
public:
    Slot();
    void init(int id) { m_id = id; }
    void glyph(unsigned short gid) { m_glyphid = gid; }
    unsigned short gid() { return m_glyphid; }
    void next(int index) { m_next = index; }
    int next() { return m_next; }
    void prev(int index) { m_prev = index; }
    void origin(Position &pos) { m_position = pos + m_shift + m_kern; }
    Position origin() { return m_position; }
    float advance(Font *font) { return font->advance(m_glyphid) + m_kern.x; }
    void before(int index) { m_before = index; }
    int before() { return m_before; }
    void after(int index) { m_after = index; }
    int after() { return m_after; }
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
};

#endif // SLOT_INCLUDE

