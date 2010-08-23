#pragma once

#include "Main.h"

#include <cassert>

//#define USE_GRLIST
#ifdef USE_GRLIST
#include "GrList.h"
#else
#include <vector>
#endif

#include "Slot.h"
#include "CharInfo.h"
#include "Features.h"
#include "XmlTraceLog.h"
#include "Silf.h"

namespace org { namespace sil { namespace graphite { namespace v2 {

#ifdef __GNUC__
// use the GNU CXX extension malloc_allocator to avoid new/delete
#include <ext/malloc_allocator.h>
typedef std::vector<Features, __gnu_cxx::malloc_allocator<Features> > FeatureList;
typedef std::vector<Slot*, __gnu_cxx::malloc_allocator<Slot*> > SlotRope;
typedef std::vector<uint16 *, __gnu_cxx::malloc_allocator<uint16 *> > AttributeRope;
#else
// standard allocator for other platforms
typedef std::vector<Features> FeatureList;
typedef std::vector<Slot *> SlotRope;
typedef std::vector<uint16 *> AttributeRope;
#endif

class GrFace;
class GrSegment
{
public:
    unsigned int length() const { return m_numGlyphs; }
    Position advance() const { return m_advance; }
    void runGraphite() { if (m_silf) m_face->runGraphite(this, m_silf); };
    void chooseSilf(uint32 script) { m_silf = m_face->chooseSilf(script); }
    const Silf *silf() const { return m_silf; }
    CharInfo *charinfo(unsigned int index) const { return index < m_numCharinfo ? m_charinfo + index : NULL; }
    int8 dir() const { return m_dir; }

    GrSegment(unsigned int numchars, const GrFace* face, uint32 script, int dir);
    ~GrSegment();
    Slot *first() { return m_first; }
    void first(Slot *p) { m_first = p; }
    Slot *last() { return m_last; }
    void last(Slot *p) { m_last = p; }
    void appendSlot(int i, int cid, int gid, int fid, int bw);
    Slot *newSlot();
    void freeSlot(Slot *);
    void positionSlots(const GrFont *font, Slot *iStart = NULL, Slot *iEnd = NULL);
    void append(const GrSegment &other);
    uint16 getClassGlyph(uint16 cid, uint16 offset) const { return m_silf->getClassGlyph(cid, offset); }
    uint16 findClassIndex(uint16 cid, uint16 gid) const { return m_silf->findClassIndex(cid, gid); }
    int addFeatures(const Features& feats) { m_feats.push_back(feats); return m_feats.size() - 1; }
    uint16 getFeature(int index, uint8 findex) const { const FeatureRef* pFR=m_face->feature(findex); if (!pFR) return 0; else return pFR->getFeatureVal(m_feats[index]); }
    void dir(int8 val) { m_dir = val; }
    uint16 glyphAttr(uint16 gid, uint8 gattr) const { return m_face->glyphAttr(gid, gattr); }
    uint16 getGlyphMetric(Slot *iSlot, uint8 metric, uint8 attrLevel) const {
        if (attrLevel > 0)
        {
            Slot *is = findRoot(iSlot);
            return is->clusterMetric(this, metric, attrLevel);
        }
        else
            return m_face->getGlyphMetric(iSlot->gid(), metric);
    }
    float glyphAdvance(uint16 gid) const { return m_face->getAdvance(gid, 1.0); }
    const Rect &theGlyphBBoxTemporary(uint16 gid) const { return m_face->theBBoxTemporary(gid); }   //warning value may become invalid when another glyph is accessed
    Slot *findRoot(Slot *is) const { return is->attachTo() ? is : findRoot(is->attachTo()); }
    int numAttrs() { return m_silf->numUser(); }

    CLASS_NEW_DELETE

#ifndef DISABLE_TRACING
    void logSegment(SegmentHandle::encform enc, const void* pStart, size_t nChars) const;
    void logSegment() const;
#endif

private:
    friend class SegmentHandle ;
    void read_text(const GrFace *face, const FeaturesHandle& pFeats/*must not be isNull*/, SegmentHandle::encform enc, const void*pStart, size_t nChars);
    void prepare_pos(const GrFont *font);
    void finalise(const GrFont *font);
  
private:
    SlotRope m_slots;           // std::vector of slot buffers
    Slot *m_freeSlots;          // linked list of free slots
    Slot *m_first;              // first slot in segment
    Slot *m_last;               // last slot in segment
    unsigned int m_bufSize;     // how big a buffer to create when need more slots
    unsigned int m_numGlyphs;
    AttributeRope m_userAttrs;  // std::vector of userAttrs buffers
    CharInfo *m_charinfo;       // character info, one per input character
    unsigned int m_numCharinfo; // size of the array and number of input characters

    const GrFace *m_face;       // GrFace
    const Silf *m_silf;
    Position m_advance;         // whole segment advance
    Rect m_bbox;                // ink box of the segment
    int8 m_dir;
    FeatureList m_feats;	// feature settings referenced by charinfos in this segment

private:		//defensive on m_charinfo
    GrSegment(const GrSegment&);
    GrSegment& operator=(const GrSegment&);
};

}}}} // namespace
