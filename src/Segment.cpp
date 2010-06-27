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
    
    delete[] m_charinfo;
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


class NoLimit
{
public:
    static bool inRange(const void* p) { return true; }
};

class Utf8Consumer
{
private:
      static const int utf8_sz_lut[16];
      static const byte utf8_mask_lut[5];

public:
      Utf8Consumer(const uint8* pCharStart) : m_pCharStart(pCharStart) {}
  
      template <class LIMIT>
      inline bool consumeChar(const LIMIT& limit, uchar_t* pRes)			//At start, limit.inRange(m_pCharStart) is true. return value is iff character contents does not go past limit
      {
	const size_t    seq_sz = utf8_sz_lut[*m_pCharStart >> 4];
	if (seq_sz==0) {
	    *pRes = 0xFFFD;
	    return true;			//this is an error. But carry on anyway?
	}
	
	if (!limit.inRange(m_pCharStart+(seq_sz-1))) {
	    return false;
	}
	
	*pRes = *m_pCharStart ^ utf8_mask_lut[seq_sz];
	
	switch(seq_sz) {      
	    case 4:     *pRes <<= 6; *pRes |= *++m_pCharStart & 0x7F;
	    case 3:     *pRes <<= 6; *pRes |= *++m_pCharStart & 0x7F;
	    case 2:     *pRes <<= 6; *pRes |= *++m_pCharStart & 0x7F; break;
	    case 1: default:    break;
	}
	++m_pCharStart; 
	return true;
      }	
  
private:
      const uint8 *m_pCharStart;
};

/*static*/ const int Utf8Consumer::utf8_sz_lut[16] = {1,1,1,1,1,1,1,        // 1 byte
                                          0,0,0,0,  // trailing byte
                                          2,2,            // 2 bytes
                                          3,                 // 3 bytes
                                          4};                // 4 bytes

/*static*/ const byte Utf8Consumer::utf8_mask_lut[5] = {0x80,0x00,0xC0,0xE0,0xF0};


class Utf16Consumer
{
private:
    static const int SURROGATE_OFFSET = 0x10000 - (0xD800 << 10) - 0xDC00;

public:
      Utf16Consumer(const uint16* pCharStart) : m_pCharStart(pCharStart) {}
  
      template <class LIMIT>
      inline bool consumeChar(const LIMIT& limit, uchar_t* pRes)			//At start, limit.inRange(m_pCharStart) is true. return value is iff character contents does not go past limit
      {
	  *pRes = *m_pCharStart;
	  if (*pRes > 0xDBFF || 0xD800 > *pRes)
	      return true;

	  if (!limit.inRange(m_pCharStart+1)) {
	      return false;
	  }

	  uchar_t ul = *++m_pCharStart;
	  if (0xDC00 > ul || ul > 0xDFFF) {
	      *pRes = 0xFFFD;
	      return true; 			//this is an error. But carry on anyway?
	  }
	  *pRes =  (*pRes<<10) + ul - SURROGATE_OFFSET;
	  return true;
      }

private:
      const uint16 *m_pCharStart;
};


namespace {

inline uchar_t consume_utf32(const uint32 **pp) {
    return *((*pp)++);
}

} // end of private namespace


class SlotBuilder
{
public:
      SlotBuilder(const LoadedFace *face2, const FeaturesHandle& pFeats/*must not be IsNull*/, Segment* pDest2)
      :	  face(face2), 
	  pDest(pDest2), 
	  ctable(TtfUtil::FindCmapSubtable(face2->getTable(tagCmap, NULL), 3, -1)), 
	  fid(pDest2->addFeatures(*pFeats.ptr())),
	  m_nCharsProcessed(0) 
      {
      }	  

      bool processChar(uchar_t cid/*unicode character*/)		//return value indicates if should stop processing
      {
	  unsigned int gid = TtfUtil::Cmap31Lookup(ctable, cid);
          pDest->appendSlot(m_nCharsProcessed, cid, gid ? gid : face->findPseudo(cid), fid);
	  ++m_nCharsProcessed;
	  return true;
      }


private:
      const LoadedFace *face;
      Segment *pDest;
      const void *const   ctable;
      const unsigned int fid;
public:
    size_t m_nCharsProcessed ;
};


void Segment::read_text(const LoadedFace *face, const FeaturesHandle& pFeats/*must not be IsNull*/, const ITextSource *txt, size_t numchars)
{
    SlotBuilder slotBuilder(face, pFeats, this);
    const void *        pChar = txt->get_utf_buffer_begin();
    uchar_t             cid;
    NoLimit limit;
    
    switch (txt->utfEncodingForm()) {
        case ITextSource::kutf8 : {
	    Utf8Consumer consumer(static_cast<const uint8 *>(pChar));
            for (; slotBuilder.m_nCharsProcessed < numchars;) {
		if (!consumer.consumeChar(limit, &cid))
		    break;
		if (!slotBuilder.processChar(cid))
		    break;
            }
            break;
        }
        case ITextSource::kutf16: {
            Utf16Consumer consumer(static_cast<const uint16 *>(pChar));
            for (; slotBuilder.m_nCharsProcessed < numchars;) {
		if (!consumer.consumeChar(limit, &cid))
		    break;
		if (!slotBuilder.processChar(cid))
		    break;
            }
            break;
        }
        case ITextSource::kutf32 : default: {
            const uint32 * p = static_cast<const uint32 *>(pChar);
            for (; slotBuilder.m_nCharsProcessed < numchars;) {
                cid = consume_utf32(&p);
		if (!slotBuilder.processChar(cid))
		    break;
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



