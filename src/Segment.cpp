#include <string.h>
#include <stdlib.h>

#include "Segment.h"
#include "graphiteng/IFont.h"
#include "CharInfo.h"
#include "Slot.h"
#include "Main.h"

Segment::Segment(int numchars, FontFace *face) :
        m_numGlyphs(numchars),
        m_numCharinfo(numchars),
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
    m_numGlyphs += other.m_numGlyphs;
    m_advance = m_advance + other.m_advance;
    m_bbox = m_bbox.widen(bbox);
}

void Segment::appendSlot(int id, int cid, int gid)
{
    m_charinfo[id].init(cid, id);
    
    m_slots[id].glyph(gid);
    m_slots[id].before(id);
    m_slots[id].after(id);
}

void Segment::positionSlots(FontImpl *font)
{
    Position currpos;
    Slot *s;

    for (int i = 0; i < m_numGlyphs; i++)
    {
        s = &(m_slots[i]);
        if (s->isBase())
        {
            float cMin = currpos.x;
            float cMax = currpos.x;
            s->finalise(this, font, currpos, &cMin, &cMax);
            currpos = Position(cMax, 0);
        }
    }
    m_advance = currpos;
}
