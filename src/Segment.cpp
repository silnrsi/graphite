#include <string.h>
#include <stdlib.h>

#include "Segment.h"
#include "graphiteng/IFont.h"
#include "CharInfo.h"
#include "Slot.h"
#include "Main.h"

Segment::Segment(int numchars, IFontImpl *font, IFaceImpl *face) :
        m_numCharinfo(numchars),
        m_font(font),
        m_face(face),
        m_slots(numchars),
        m_charinfo(new CharInfo[numchars]),
        m_bbox(Rect(Position(0, 0), Position(0, 0)))
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
}

void Segment::append(const Segment &other)
{
    Rect bbox = other.m_bbox + m_advance;

    m_slots.insert(m_slots.end(), other.m_slots.begin(), other.m_slots.end());
    m_charinfo = (CharInfo *)realloc(m_charinfo, other.m_numCharinfo * sizeof(CharInfo));
    memcpy(m_charinfo + m_numCharinfo, other.m_charinfo, other.m_numCharinfo * sizeof(CharInfo));
    for (int i = 0; i < other.m_numCharinfo; i++)
    { m_charinfo[m_numCharinfo + i].update(m_numCharinfo); }
    m_numCharinfo += other.m_numCharinfo;
    m_advance = m_advance + other.m_advance;
    m_bbox = m_bbox.widen(bbox);
}

void Segment::initslots(int index, int cid, int gid)
{
    CharInfo *c = m_charinfo + index;
    m_charinfo[index].init(cid, index);
    m_slots[index].init(index);
    m_slots[index].glyph(gid);
    m_slots[index].before(index);
    m_slots[index].after(index);
}

void Segment::positionSlots()
{
    std::vector<Slot>::iterator s;
    Position currpos;

    for (s = m_slots.begin(); s != m_slots.end(); s++)
    {
        s->origin(currpos);
        currpos = currpos + Position(s->advance(m_font), 0);
    }
    m_advance = currpos;
}

