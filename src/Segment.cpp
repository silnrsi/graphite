#include <string.h>
#include <stdlib.h>

#include "Segment.h"
#include "Font.h"
#include "CharInfo.h"
#include "Slot.h"
#include "Misc.h"

Segment::Segment(int numchars, Font *font) :
        m_maxSlots(numchars),
        m_numCharinfo(numchars),
        m_font(font),
        m_slots(new Slot[numchars]),
        m_charinfo(new CharInfo[numchars]),
        m_bbox(Rect(Position(0, 0), Position(0, 0)))
{

}

Segment::~Segment()
{
    delete[] m_slots;
    delete[] m_charinfo;
}

Segment::Segment(const Segment &other)
{
    memcpy(this, &other, sizeof(Segment));
    m_charinfo = new CharInfo[m_numCharinfo];
    memcpy(m_charinfo, other.m_charinfo, m_numCharinfo * sizeof(CharInfo));
    m_slots = new Slot[m_maxSlots];
    memcpy(m_slots, other.m_slots + 1, m_numSlots * sizeof(Slot));
    m_slots--;
}

void Segment::growSlots(int num)
{
    m_slots = (Slot *)realloc((Slot *)m_slots + 1, (m_maxSlots + num) * sizeof(Slot)) - 1;
    memset(m_slots + m_maxSlots + 1, num * sizeof(Slot), 0);
    m_maxSlots += num;
}

void Segment::append(const Segment &other)
{
    Rect bbox = other.m_bbox + m_advance;

    if (m_maxSlots < m_numSlots + other.m_numSlots)
    { growSlots(other.m_numSlots); }
    memcpy(m_slots + m_numSlots + 1, other.m_slots + 1, other.m_numSlots * sizeof(Slot));
    for (int i = 1; i <= other.m_numSlots; i++)
    { m_slots[m_numSlots + i].update(m_numSlots, m_numCharinfo, m_advance); }
    m_slots[m_last].next(other.m_first + m_numSlots);
    m_last = other.m_last + m_numSlots;
    m_charinfo = (CharInfo *)realloc(m_charinfo, other.m_numCharinfo * sizeof(CharInfo));
    memcpy(m_charinfo + m_numCharinfo, other.m_charinfo, other.m_numCharinfo * sizeof(CharInfo));
    for (int i = 0; i < other.m_numCharinfo; i++)
    { m_charinfo[m_numCharinfo + i].update(m_numCharinfo, m_numSlots); }
    m_advance = m_advance + other.m_advance;
    m_bbox = m_bbox.widen(bbox);
}

void Segment::initslots(int index, int cid, int gid)
{
    CharInfo *c = m_charinfo + index;
    m_charinfo[index].init(cid, index + 1);
    index++;                                // slots start at 1
    m_slots[index].init(index);
    m_slots[index].glyph(gid);
    if (m_last)
    {
        m_slots[index].prev(m_last);
        m_slots[m_last].next(index);
    }
    if (!m_first) m_first = index;
    m_last = index;
}

void Segment::positionSlots()
{
    Slot *s;
    int i;
    Position currpos;

    for ((i = m_first), s = m_slots + i; i != 0; (i = s->next()), s = m_slots + i)
    {
        s->origin(currpos);
        currpos = Position(s->advance(m_font), 0);
    }
}

