#include <string.h>
#include <stdlib.h>

#include "Segment.h"
#include "graphiteng/IFont.h"
#include "graphiteng/ITextSource.h"
#include "CharInfo.h"
#include "Slot.h"
#include "Main.h"
#include "XmlTraceLog.h"
#include "graphiteng/SegmentHandle.h"

Segment::Segment(unsigned int numchars, const LoadedFace *face) :
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
    delete[] m_charinfo;
}

#if 0		//unsafe - memcpying vectors will lead to problems when the destructors are run
Segment::Segment(const Segment &other)
{
    memcpy(this, &other, sizeof(Segment));
    m_charinfo = (CharInfo *)(operator new(m_numCharinfo * sizeof(CharInfo)));
    memcpy(m_charinfo, other.m_charinfo, m_numCharinfo * sizeof(CharInfo));
}
#endif

void Segment::append(const Segment &other)
{
    Rect bbox = other.m_bbox + m_advance;

    m_slots.insert(m_slots.end(), other.m_slots.begin(), other.m_slots.end());
    CharInfo* pNewCharInfo = new CharInfo[m_numCharinfo+other.m_numCharinfo];		//since CharInfo has no constructor, this doesn't do much
    for (unsigned int i=0 ; i<m_numCharinfo ; ++i)
	pNewCharInfo[i] = m_charinfo[i];
    
    m_charinfo = pNewCharInfo;
    pNewCharInfo += m_numCharinfo ;
    for (unsigned int i=0 ; i<m_numCharinfo ; ++i)
    {
	pNewCharInfo[i] = other.m_charinfo[i];
	pNewCharInfo[i].update(m_numCharinfo);
    }
 
    m_numCharinfo += other.m_numCharinfo;
    m_numGlyphs += other.m_numGlyphs;
    m_advance = m_advance + other.m_advance;
    m_bbox = m_bbox.widen(bbox);
}

void Segment::appendSlot(int id, int cid, int gid, int iFeats)
{
    m_charinfo[id].init(cid, id);
    m_charinfo[id].feats(iFeats);
    
    m_slots[id].child(this, -1);
    m_slots[id].glyph(gid);
    m_slots[id].originate(id);
    m_slots[id].before(id);
    m_slots[id].after(id);
}

void Segment::positionSlots(const LoadedFont *font)
{
    Position currpos;
    Slot *s;

    for (unsigned int i = 0; i < m_numGlyphs; i++)
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
void Segment::logSegment(const ITextSource & textSrc) const
{
    if (XmlTraceLog::get().active())
    {
        XmlTraceLog::get().openElement(ElementSegment);
        XmlTraceLog::get().addAttribute(AttrLength, length());
        XmlTraceLog::get().addAttribute(AttrAdvanceX, advance().x);
        XmlTraceLog::get().addAttribute(AttrAdvanceY, advance().y);
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
            for (size_t j = 0; j < textSrc.getLength(); ++j)
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
            for (size_t j = 0; j < textSrc.getLength(); ++j)
            {
                XmlTraceLog::get().writeUnicode(
                    reinterpret_cast<const uint32 *>(textSrc.get_utf_buffer_begin())[j]);
            }
            break;
        }
        XmlTraceLog::get().closeElement(ElementText);
        for (unsigned int i = 0; i < length(); i++)
        {
            XmlTraceLog::get().openElement(ElementSlot);
            XmlTraceLog::get().addAttribute(AttrGlyphId, (*this)[i].gid());
            XmlTraceLog::get().addAttribute(AttrX, (*this)[i].origin().x);
            XmlTraceLog::get().addAttribute(AttrY, (*this)[i].origin().y);
            XmlTraceLog::get().addAttribute(AttrBefore, (*this)[i].before());
            XmlTraceLog::get().addAttribute(AttrAfter, (*this)[i].after());
            XmlTraceLog::get().closeElement(ElementSlot);
        }
        XmlTraceLog::get().closeElement(ElementSegment);
    }
}

#endif

typedef unsigned int uchar_t;
namespace {
const int utf8_sz_lut[16] = {1,1,1,1,1,1,1,        // 1 byte
                                          0,0,0,0,  // trailing byte
                                          2,2,            // 2 bytes
                                          3,                 // 3 bytes
                                          4};                // 4 bytes
const byte utf8_mask_lut[5] = {0x80,0x00,0xC0,0xE0,0xF0};

inline uchar_t consume_utf8(const uint8 *&p) {
    const size_t    seq_sz = utf8_sz_lut[*p >> 4];
    uchar_t         uc = *p ^ utf8_mask_lut[seq_sz];
    
    switch(seq_sz) {
        case 4:     uc <<= 6; uc |= *++p & 0x7F;
        case 3:     uc <<= 6; uc |= *++p & 0x7F;
        case 2:     uc <<= 6; uc |= *++p & 0x7F; break;
        case 1:     break;
        case 0:     uc = 0xFFFD; break;
    }
    ++p; return uc;
}

const int SURROGATE_OFFSET = 0x10000 - (0xD800 << 10) - 0xDC00;
inline uchar_t consume_utf16(const uint16 *&p) {
    const uchar_t   uh = *p, ul = *++p;
    
    if (0xD800 > uh || uh > 0xDBFF)
        return uh;
    ++p;
    if (0xDC00 > ul || ul > 0xDFFF) {
        return 0xFFFD;
    }
    return (uh<<10) + ul - SURROGATE_OFFSET;
}

} // end of private namespace



void Segment::read_text(const LoadedFace *face, const FeaturesHandle& pFeats/*must not be IsNull*/, const ITextSource *txt, size_t numchars)
{
    const void *const   cmap = face->getTable(tagCmap, NULL);
    const void *const   ctable = TtfUtil::FindCmapSubtable(cmap, 3, -1);
    const void *        pChar = txt->get_utf_buffer_begin();
    uchar_t             cid;
    unsigned int        gid;
    unsigned int	fid = addFeatures(*pFeats.Ptr());
    
    switch (txt->utfEncodingForm()) {
        case ITextSource::kutf8 : {
            const uint8 * p = static_cast<const uint8 *>(pChar);
            for (size_t i = 0; i < numchars; ++i) {
                cid = consume_utf8(p);
                gid = TtfUtil::Cmap31Lookup(ctable, cid);
                appendSlot(i, cid, gid ? gid : face->findPseudo(cid), fid);
            }
            break;
        }
        case ITextSource::kutf16: {
            const uint16 * p = static_cast<const uint16 *>(pChar);
            for (size_t i = 0; i < numchars; ++i) {
                cid = consume_utf16(p);
                gid = TtfUtil::Cmap31Lookup(ctable, cid);
                appendSlot(i, cid, gid ? gid : face->findPseudo(cid), fid);
            }
            break;
        }
        case ITextSource::kutf32 : default: {
            const uint32 * p = static_cast<const uint32 *>(pChar);
            for (size_t i = 0; i < numchars; ++i) {
                cid = *p++;
                gid = TtfUtil::Cmap31Lookup(ctable, cid);
                appendSlot(i, cid, gid ? gid : face->findPseudo(cid), fid);
            }
            break;
        }
    }
}

void Segment::prepare_pos(const LoadedFont *font)
{
    // reorder for bidi
    // copy key changeable metrics into slot (if any);
}

void Segment::finalise(const LoadedFont *font)
{
    positionSlots(font);
}



