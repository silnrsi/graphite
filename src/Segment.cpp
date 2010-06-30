#include "processUTF.h"
#include <string.h>
#include <stdlib.h>
#include <vector>

#include "Segment.h"
#include "graphiteng/IFont.h"
#include "CharInfo.h"
#include "Slot.h"
#include "Main.h"
#include "XmlTraceLog.h"
#include "graphiteng/SegmentHandle.h"

Segment::Segment(unsigned int numchars, const LoadedFace *face, uint32 script) :
        m_numGlyphs(numchars),
        m_numCharinfo(numchars),
        m_face(face),
        m_slots(numchars),
        m_charinfo(new CharInfo[numchars]),
        m_silf(face->chooseSilf(script)),
        m_bbox(Rect(Position(0, 0), Position(0, 0)))
{
    m_userAttrs.assign(m_silf->numUser() * numchars, 0);         // initialise to 0
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
    m_slots[id].setGlyph(this, gid);
    m_slots[id].originate(id);
    m_slots[id].before(id);
    m_slots[id].after(id);
}

void Segment::positionSlots(const LoadedFont *font)
{
    Position currpos;
    Slot *s;
    float cMin = 0.;

    for (unsigned int i = 0; i < m_numGlyphs; i++)
    {
        s = &(m_slots[i]);
        if (s->isBase())
        {
            Rect bbox = Rect(currpos, currpos);
            currpos = s->finalise(this, font, &currpos, &bbox, &cMin, 0);
        }
    }
    m_advance = currpos;
}

#ifndef DISABLE_TRACING
void Segment::logSegment(SegmentHandle::encform enc, const void* pStart, size_t nChars) const
{
    if (XmlTraceLog::get().active())
    {
        XmlTraceLog::get().openElement(ElementSegment);
        XmlTraceLog::get().addAttribute(AttrLength, length());
        XmlTraceLog::get().addAttribute(AttrAdvanceX, advance().x);
        XmlTraceLog::get().addAttribute(AttrAdvanceY, advance().y);
        XmlTraceLog::get().openElement(ElementText);
        XmlTraceLog::get().addAttribute(AttrEncoding, enc);
        XmlTraceLog::get().addAttribute(AttrLength, nChars);
        switch (enc)
        {
        case SegmentHandle::kutf8:
            XmlTraceLog::get().writeText(
                reinterpret_cast<const char *>(pStart));
            break;
        case SegmentHandle::kutf16:
            for (size_t j = 0; j < nChars; ++j)
            {
                uint32 code = reinterpret_cast<const uint16 *>(pStart)[j];
                if (code >= 0xD800 && code <= 0xDBFF) // high surrogate
                {
                    j++;
                    // append low surrogate
                    code = (code << 16) + reinterpret_cast<const uint16 *>(pStart)[j];
                }
                else if (code >= 0xDC00 && code <= 0xDFFF)
                {
                    XmlTraceLog::get().warning("Unexpected low surrogate %x at %d", code, j);
                }
                XmlTraceLog::get().writeUnicode(code);
            }
            break;
        case SegmentHandle::kutf32:
            for (size_t j = 0; j < nChars; ++j)
            {
                XmlTraceLog::get().writeUnicode(
                    reinterpret_cast<const uint32 *>(pStart)[j]);
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




class SlotBuilder
{
public:
      SlotBuilder(const LoadedFace *face2, const FeaturesHandle& pFeats/*must not be isNull*/, Segment* pDest2)
      :	  face(face2), 
	  pDest(pDest2), 
	  ctable(TtfUtil::FindCmapSubtable(face2->getTable(tagCmap, NULL), 3, -1)), 
	  fid(pDest2->addFeatures(*pFeats.ptr())),
	  m_nCharsProcessed(0) 
      {
      }	  

      bool processChar(uint32 cid/*unicode character*/)		//return value indicates if should stop processing
      {
	  unsigned int gid = TtfUtil::Cmap31Lookup(ctable, cid);
          pDest->appendSlot(m_nCharsProcessed, cid, gid ? gid : face->findPseudo(cid), fid);
	  ++m_nCharsProcessed;
	  return true;
      }

      size_t charsProcessed() const { return m_nCharsProcessed; }

private:
      const LoadedFace *face;
      Segment *pDest;
      const void *const   ctable;
      const unsigned int fid;
      size_t m_nCharsProcessed ;
};


void Segment::read_text(const LoadedFace *face, const FeaturesHandle& pFeats/*must not be isNull*/, SegmentHandle::encform enc, const void* pStart, size_t nChars)
{
    SlotBuilder slotBuilder(face, pFeats, this);
    CharacterCountLimit limit(enc, pStart, nChars);
    IgnoreErrors ignoreErrors;

    processUTF(limit/*when to stop processing*/, &slotBuilder, &ignoreErrors);
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



