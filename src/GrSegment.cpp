#include "processUTF.h"
#include <string.h>
#include <stdlib.h>
#include <new>

#include "GrSegment.h"
#include "graphiteng/font.h"
#include "CharInfo.h"
#include "Slot.h"
#include "Main.h"
#include "XmlTraceLog.h"
#include "graphiteng/SegmentHandle.h"

namespace org { namespace sil { namespace graphite { namespace v2 {

GrSegment::GrSegment(unsigned int numchars, const GrFace* face, uint32 script, int textDir) :
        m_numGlyphs(numchars),
        m_numCharinfo(numchars),
        m_face(face),
        m_charinfo(new CharInfo[numchars]),
        m_silf(face->chooseSilf(script)),
        m_dir(textDir),
        m_bbox(Rect(Position(0, 0), Position(0, 0)))
{
    unsigned int i, j;
    m_bufSize = numchars + 10;
    freeSlot(newSlot());
    for (i = 0, j = 1; j < numchars; i++, j <<= 1) {}
    m_bufSize = i;                  // log2(numchars)
}

GrSegment::~GrSegment()
{
    SlotRope::iterator i;
    AttributeRope::iterator j;
    
    for (i = m_slots.begin(); i != m_slots.end(); i++)
        delete[] *i;
    for (j = m_userAttrs.begin(); j != m_userAttrs.end(); j++)
        delete[] *j;
    delete[] m_charinfo;
}

void GrSegment::append(const GrSegment &other)
{
    Rect bbox = other.m_bbox + m_advance;

    m_slots.insert(m_slots.end(), other.m_slots.begin(), other.m_slots.end());
    CharInfo* pNewCharInfo = new CharInfo[m_numCharinfo+other.m_numCharinfo];		//since CharInfo has no constructor, this doesn't do much
    for (unsigned int i=0 ; i<m_numCharinfo ; ++i)
	pNewCharInfo[i] = m_charinfo[i];
    m_last->next(other.m_first);
    other.m_last->prev(m_last);
    m_userAttrs.insert(m_userAttrs.end(), other.m_userAttrs.begin(), other.m_userAttrs.end());
    
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

void GrSegment::appendSlot(int id, int cid, int gid, int iFeats, int bw)
{
    Slot *aSlot = newSlot();
    m_charinfo[id].init(cid, id);
    m_charinfo[id].feats(iFeats);
    m_charinfo[id].breakWeight(bw);
    
    aSlot->child(NULL);
    aSlot->setGlyph(this, gid);
    aSlot->originate(id);
    aSlot->before(id);
    aSlot->after(id);
    if (m_last) m_last->next(aSlot);
    aSlot->prev(m_last);
    m_last = aSlot;
    if (!m_first) m_first = aSlot;
}

Slot *GrSegment::newSlot()
{
    if (!m_freeSlots)
    {
        int numUser = m_silf->numUser();
        Slot *newSlots = new Slot[m_bufSize];
        uint16 *newAttrs = new uint16 [numUser * m_bufSize];
        
        memset(newAttrs, 0, numUser * m_bufSize * sizeof(uint16));
        
        for (size_t i = 0; i < m_bufSize - 1; i++)
        {
            newSlots[i].next(newSlots + i + 1);
            newSlots[i].userAttrs(newAttrs + i * numUser);
        }
        newSlots[m_bufSize - 1].userAttrs(newAttrs + (m_bufSize - 1) * numUser);
        m_slots.push_back(newSlots);
        m_userAttrs.push_back(newAttrs);
        m_freeSlots = newSlots;
    }
    Slot *res = m_freeSlots;
    m_freeSlots = m_freeSlots->next();
    ::new (res) Slot;
    return res;
}

void GrSegment::freeSlot(Slot *aSlot)
{
    if (!m_freeSlots)
        aSlot->next(NULL);
    else
        aSlot->next(m_freeSlots);
    m_freeSlots = aSlot;
}
        
void GrSegment::positionSlots(const GrFont *font, Slot *iStart, Slot *iEnd)
{
    Position currpos;
    Slot *s;
    float cMin = 0.;
    Rect bbox;

    if (!iStart) iStart = m_first;
    if (!iEnd) iEnd = m_last;
    
    if (m_dir & 1)
    {
        for (s = iEnd; s != iStart->prev(); s = s->prev())
        {
            if (s->isBase())
                currpos = s->finalise(this, font, &currpos, &bbox, &cMin, 0);
        }
    }
    else
    {
        for (s = iStart; s != iEnd->next(); s = s->next())
        {
            if (s->isBase())
                currpos = s->finalise(this, font, &currpos, &bbox, &cMin, 0);
        }
    }
    if (iStart == m_first && iEnd == m_last) m_advance = currpos;
}

#ifndef DISABLE_TRACING
void GrSegment::logSegment(SegmentHandle::encform enc, const void* pStart, size_t nChars) const
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

void GrSegment::logSegment() const
{
    if (XmlTraceLog::get().active())
    {
        XmlTraceLog::get().openElement(ElementSegment);
        XmlTraceLog::get().addAttribute(AttrLength, length());
        XmlTraceLog::get().addAttribute(AttrAdvanceX, advance().x);
        XmlTraceLog::get().addAttribute(AttrAdvanceY, advance().y);
        for (Slot *i = m_first; i; i = i->next())
        {
            XmlTraceLog::get().openElement(ElementSlot);
            XmlTraceLog::get().addAttribute(AttrGlyphId, i->gid());
            XmlTraceLog::get().addAttribute(AttrX, i->origin().x);
            XmlTraceLog::get().addAttribute(AttrY, i->origin().y);
            XmlTraceLog::get().addAttribute(AttrBefore, i->before());
            XmlTraceLog::get().addAttribute(AttrAfter, i->after());
            XmlTraceLog::get().closeElement(ElementSlot);
        }
        XmlTraceLog::get().closeElement(ElementSegment);
    }
}

#endif



class SlotBuilder
{
public:
      SlotBuilder(const GrFace *face2, const FeaturesHandle& pFeats/*must not be isNull*/, GrSegment* pDest2)
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
      GrSegment *m_pDest;
      const void *const   m_ctable;
      const unsigned int m_fid;
      size_t m_nCharsProcessed ;
};


void GrSegment::read_text(const GrFace *face, const FeaturesHandle& pFeats/*must not be isNull*/, SegmentHandle::encform enc, const void* pStart, size_t nChars)
{
    SlotBuilder slotBuilder(face, pFeats, this);
    CharacterCountLimit limit(enc, pStart, nChars);
    IgnoreErrors ignoreErrors;

    processUTF(limit/*when to stop processing*/, &slotBuilder, &ignoreErrors);
}

void GrSegment::prepare_pos(const GrFont *font)
{
    // copy key changeable metrics into slot (if any);
}

void GrSegment::finalise(const GrFont *font)
{
    positionSlots(font);
}

}}}} // namespace
