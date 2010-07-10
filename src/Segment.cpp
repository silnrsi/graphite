#include "processUTF.h"
#include <string.h>
#include <stdlib.h>

#include "Segment.h"
#include "graphiteng/IFont.h"
#include "CharInfo.h"
#include "Slot.h"
#include "Main.h"
#include "XmlTraceLog.h"
#include "graphiteng/SegmentHandle.h"

namespace org { namespace sil { namespace graphite { namespace v2 {

Segment::Segment(unsigned int numchars, const GrFace* face, uint32 script, int textDir) :
        m_numGlyphs(numchars),
        m_numCharinfo(numchars),
        m_face(face),
        m_slots(numchars),
        m_charinfo(new CharInfo[numchars]),
        m_silf(face->chooseSilf(script)),
        m_dir(textDir),
        m_bbox(Rect(Position(0, 0), Position(0, 0)))
{
    m_userAttrs.assign(m_silf->numUser() * numchars, 0);         // initialise to 0
}

Segment::~Segment()
{
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

void Segment::appendSlot(int id, int cid, int gid, int iFeats, int bw)
{
    m_charinfo[id].init(cid, id);
    m_charinfo[id].feats(iFeats);
    m_charinfo[id].breakWeight(bw);
    
    m_slots[id].child(this, -1);
    m_slots[id].setGlyph(this, gid);
    m_slots[id].originate(id);
    m_slots[id].before(id);
    m_slots[id].after(id);
}

void Segment::positionSlots(const GrFont *font)
{
    Position currpos;
    Slot *s;
    float cMin = 0.;
    Rect bbox;

    if (m_dir & 1)
    {
        for (int i = m_numGlyphs - 1; i >= 0; i--)
        {
            s = &(m_slots[i]);
            if (s->isBase())
                currpos = s->finalise(this, font, &currpos, &bbox, &cMin, 0);
        }
    }
    else
    {
        for (unsigned int i = 0; i < m_numGlyphs; i++)
        {
            s = &(m_slots[i]);
            if (s->isBase())
                currpos = s->finalise(this, font, &currpos, &bbox, &cMin, 0);
        }
    }
    m_advance = currpos;
    if (cMin != 0)
    {
        Position adj = Position(-cMin, 0);
        for (unsigned int i = 0; i < m_numGlyphs; i++)
            m_slots[i].positionShift(adj);
        m_advance += adj;
    }
}

void Segment::positionSlots(int iSlot, int *iStart, int *iFinish, Position *endPos)
{
    Position currpos;
    float cMin = 0.;
    Rect bbox;
    int start;
    int end;    // NB last slot we process not one past
    
    if (*iFinish > -1 && iSlot > *iFinish)
    {
        currpos = *endPos;
        start = *iFinish + 1;
        end = iSlot;
    }
    else if (iSlot < *iStart)
    {
        start = iSlot;
        end = *iStart - 1;
    }
    else if (*iStart == -1)
    {
        *iStart = start = iSlot;
        *iFinish = end = iSlot;
    }
    else
        return;
    
    start = findRoot(start);
    for ( ; end + 1 < static_cast<int>(m_numGlyphs) && m_slots[end + 1].attachTo() != -1; end++ ) {}
    if (m_dir)
    {
        for (int i = end; i >= start; i--)
        {
            Slot *s = &(m_slots[i]);
            if (s->isBase())
                currpos = s->finalise(this, NULL, &currpos, &bbox, &cMin, 0);
        }
    }
    else
    {
        for (int i = start; i <= end; i++)
        {
            Slot *s = &(m_slots[i]);
            if (s->isBase())
                currpos = s->finalise(this, NULL, &currpos, &bbox, &cMin, 0);
        }
    }
    
    if (start < *iStart)
    {
        Position lShift = Position() - currpos;
        for (int i = start; i < *iStart; i++)
            m_slots[i].positionShift(lShift);
        *iStart = start;
    }
    
    if (end >= *iFinish)
    {
        *endPos = currpos;
        *iFinish = end;
    }
}
    
#ifndef DISABLE_TRACING
void Segment::logSegment(SegmentHandle::encform enc, const void* pStart, size_t nChars) const
{
    if (XmlTraceLog::get().active())
    {
        if (pStart)
        {
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
        }
        logSegment();
    }
}

void Segment::logSegment() const
{
    if (XmlTraceLog::get().active())
    {
        XmlTraceLog::get().openElement(ElementSegment);
        XmlTraceLog::get().addAttribute(AttrLength, length());
        XmlTraceLog::get().addAttribute(AttrAdvanceX, advance().x);
        XmlTraceLog::get().addAttribute(AttrAdvanceY, advance().y);
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
      SlotBuilder(const GrFace *face2, const FeaturesHandle& pFeats/*must not be isNull*/, Segment* pDest2)
      :	  m_face(face2), 
	  m_pDest(pDest2), 
	  m_ctable(TtfUtil::FindCmapSubtable(face2->getTable(tagCmap, NULL), 3, -1)), 
	  m_fid(pDest2->addFeatures(*pFeats.ptr())),
	  m_nCharsProcessed(0) 
      {
      }	  

      bool processChar(uint32 cid/*unicode character*/)		//return value indicates if should stop processing
      {
          uint16 realgid = 0;
	  uint16 gid = TtfUtil::Cmap31Lookup(m_ctable, cid);
          if (!gid)
              gid = m_face->findPseudo(cid);
          int16 bw = m_face->glyphAttr(gid, m_pDest->silf()->aBreak());
          m_pDest->appendSlot(m_nCharsProcessed, cid, gid, m_fid, bw);
	  ++m_nCharsProcessed;
	  return true;
      }

      size_t charsProcessed() const { return m_nCharsProcessed; }

private:
      const GrFace *m_face;
      Segment *m_pDest;
      const void *const   m_ctable;
      const unsigned int m_fid;
      size_t m_nCharsProcessed ;
};


void Segment::read_text(const GrFace *face, const FeaturesHandle& pFeats/*must not be isNull*/, SegmentHandle::encform enc, const void* pStart, size_t nChars)
{
    SlotBuilder slotBuilder(face, pFeats, this);
    CharacterCountLimit limit(enc, pStart, nChars);
    IgnoreErrors ignoreErrors;

    processUTF(limit/*when to stop processing*/, &slotBuilder, &ignoreErrors);
}

void Segment::prepare_pos(const GrFont *font)
{
    // copy key changeable metrics into slot (if any);
}

void Segment::finalise(const GrFont *font)
{
    positionSlots(font);
}

}}}} // namespace
