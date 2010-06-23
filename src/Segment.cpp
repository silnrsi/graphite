#include <string.h>
#include <stdlib.h>

#include "Segment.h"
#include "graphiteng/IFont.h"
#include "graphiteng/ITextSource.h"
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

#ifndef DISABLE_TRACING
void logSegment(const ITextSource & textSrc, const ISegment & seg)
{
    if (XmlTraceLog::get().active())
    {
        XmlTraceLog::get().openElement(ElementSegment);
        XmlTraceLog::get().addAttribute(AttrLength, seg.length());
        XmlTraceLog::get().addAttribute(AttrAdvanceX, seg.advance().x);
        XmlTraceLog::get().addAttribute(AttrAdvanceY, seg.advance().y);
        XmlTraceLog::get().openElement(ElementText);
        XmlTraceLog::get().addAttribute(AttrEncoding, textSrc.utfEncodingForm());
        XmlTraceLog::get().addAttribute(AttrLength, textSrc.getLength());
        switch (textSrc.utfEncodingForm())
        {
        case ITextSource::kutf8:
            XmlTraceLog::get().writeText(
                reinterpret_cast<const char *>(textSrc.get_utf_buffer_begin()));
            break;
        case ITextSource::kutf16:
            for (int j = 0; j < textSrc.getLength(); j++)
            {
                uint32 code = reinterpret_cast<const uint16 *>(textSrc.get_utf_buffer_begin())[j];
                if (code >= 0xD800 && code <= 0xDBFF) // high surrogate
                {
                    j++;
                    // append low surrogate
                    code = (code << 16) + reinterpret_cast<const uint16 *>(textSrc.get_utf_buffer_begin())[j];
                }
                else if (code >= 0xDC00 && code <= 0xDFFF)
                {
                    XmlTraceLog::get().warning("Unexpected low surrogate %x at %d", code, j);
                }
                XmlTraceLog::get().writeUnicode(code);
            }
            break;
        case ITextSource::kutf32:
            for (int j = 0; j < textSrc.getLength(); j++)
            {
                XmlTraceLog::get().writeUnicode(
                    reinterpret_cast<const uint32 *>(textSrc.get_utf_buffer_begin())[j]);
            }
            break;
        }
        XmlTraceLog::get().closeElement(ElementText);
        for (int i = 0; i < seg.length(); i++)
        {
            XmlTraceLog::get().openElement(ElementSlot);
            XmlTraceLog::get().addAttribute(AttrGlyphId, seg[i].gid());
            XmlTraceLog::get().addAttribute(AttrX, seg[i].origin().x);
            XmlTraceLog::get().addAttribute(AttrY, seg[i].origin().y);
            XmlTraceLog::get().addAttribute(AttrBefore, seg[i].before());
            XmlTraceLog::get().addAttribute(AttrAfter, seg[i].after());
            XmlTraceLog::get().closeElement(ElementSlot);
        }
        XmlTraceLog::get().closeElement(ElementSegment);
    }
}

#endif
