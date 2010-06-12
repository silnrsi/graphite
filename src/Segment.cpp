#include <string.h>
#include <stdlib.h>

#include "Segment.h"
#include "graphiteng/IFont.h"
#include "CharInfo.h"
#include "Slot.h"
#include "Main.h"

Segment::Segment(int numchars, IFont *font) :
        m_maxSlots(numchars + 1),
        m_numCharinfo(numchars),
        m_font(font),
        m_slots(new Slot[numchars + 1]),
        m_charinfo(new CharInfo[numchars]),
        m_bbox(Rect(Position(0, 0), Position(0, 0))),
        m_last(0),
        m_first(0)
{

}

Segment::~Segment()
{
//    delete[] m_slots;
//    delete[] m_charinfo;
}

Segment::Segment(const Segment &other)
{
    memcpy(this, &other, sizeof(Segment));
    m_charinfo = (CharInfo *)(operator new(m_numCharinfo * sizeof(CharInfo)));
    memcpy(m_charinfo, other.m_charinfo, m_numCharinfo * sizeof(CharInfo));
    m_slots = (Slot *)(operator new(m_maxSlots * sizeof(Slot)));
    memcpy(m_slots, other.m_slots + 1, m_numSlots * sizeof(Slot));
    m_slots--;
}

void Segment::growSlots(int num)
{
    Slot *newslots = (Slot *)(operator new((m_maxSlots + num) * sizeof(Slot)));
    m_maxSlots += num;
    memcpy(newslots, m_slots + 1, m_numSlots * sizeof(Slot));
    memset(newslots + m_numSlots + 1, (m_maxSlots - m_numSlots) * sizeof(Slot), 0);     // or is it m_numSlots + 1?
    delete m_slots;
    m_slots = newslots;
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
    m_charinfo[index].init(cid, m_numSlots + 1);
    m_numSlots++;
    m_slots[m_numSlots].init(m_numSlots);
    m_slots[m_numSlots].glyph(gid);
    m_slots[m_numSlots].before(index);
    m_slots[m_numSlots].after(index);
    if (m_last)
    {
        m_slots[m_numSlots].prev(m_last);
        m_slots[m_last].next(m_numSlots);
    }
    if (!m_first) m_first = m_numSlots;
    m_last = m_numSlots;
}

void Segment::positionSlots()
{
    Slot *s;
    int i;
    Position currpos;

    for ((i = m_first), s = m_slots + i; i != 0; (i = s->next()), s = m_slots + i)
    {
        s->origin(currpos);
        currpos = currpos + Position(s->advance(m_font), 0);
    }
    m_advance = currpos;
}

