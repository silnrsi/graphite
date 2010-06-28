#ifndef SEGMENT_INCLUDE
#define SEGMENT_INCLUDE

#include "Main.h"

#include <cassert>
#include <vector>

#include "Slot.h"
#include "CharInfo.h"
#include "Features.h"
#include "XmlTraceLog.h"

class Silf;
class LoadedFace;
class Segment
{
public:
    unsigned int length() const { return m_numGlyphs; }
    Position advance() const { return m_advance; }
    Slot & operator[] (unsigned int index) {
#ifdef ENABLE_DEEP_TRACING
        if (static_cast<size_t>(index) >= m_slots.size())
        {
            XmlTraceLog::get().warning("Unexpectedly extending Segment size from %d to %d",
                m_slots.size(), index + 1);
            // temp hack
            m_slots.reserve(index+1);
            while (m_slots.size() <= static_cast<size_t>(index))
            {
                Slot s;
                s.glyph(0);
                s.before(-1);
                s.after(-1);
                m_slots.push_back(s);
            }
        }
#endif
        return m_slots[index];
    }
    const Slot & operator[] (unsigned int index) const { return m_slots[index]; }
    void runGraphite() { if (m_silf) m_face->runGraphite(this, m_silf); };
    void chooseSilf(uint32 script) { m_silf = m_face->chooseSilf(script); }
    CharInfo *charinfo(unsigned int index) const { return index < m_numCharinfo ? m_charinfo + index : NULL; }
    virtual int8 dir() const { return m_dir; }

    Segment(unsigned int numSlots, const LoadedFace *face);
    ~Segment();
    void appendSlot(int i, int cid, int gid, int fid);
    void positionSlots(const LoadedFont *font);
    void append(const Segment &other);
    void insertSlot(int index) { m_slots.insert(m_slots.begin() + index, Slot()); m_numGlyphs++; }
    void deleteSlot(int index) { m_slots.erase(m_slots.begin() + index); m_numGlyphs--; }
    uint16 getClassGlyph(uint16 cid, uint16 offset) const { return m_silf->getClassGlyph(cid, offset); }
    uint16 findClassIndex(uint16 cid, uint16 gid) const { return m_silf->findClassIndex(cid, gid); }
    int addFeatures(const Features& feats) { m_feats.push_back(feats); return m_feats.size() - 1; }
    uint16 getFeature(int index, uint8 findex) const { const FeatureRef* pFR=m_face->feature(findex); if (!pFR) return 0; else return pFR->getFeatureVal(m_feats[index]); }
    void dir(int8 val) { m_dir = val; }
    uint16 glyphAttr(uint16 gid, uint8 gattr) const { return m_face->glyphAttr(gid, gattr); }
    uint16 getGlyphMetric(uint16 gid, uint8 metric) const { return m_face->getGlyphMetric(gid, metric); }

#ifndef DISABLE_TRACING
    void logSegment(SegmentHandle::encform enc, const void* pStart, size_t nChars) const;
#endif

private:
    friend class SegmentHandle ;
    void read_text(const LoadedFace *face, const FeaturesHandle& pFeats/*must not be isNull*/, SegmentHandle::encform enc, const void*pStart, size_t nChars);
    void prepare_pos(const LoadedFont *font);
    void finalise(const LoadedFont *font);
  
private:
    std::vector<Slot> m_slots;
    std::vector<uint16> m_user;
    unsigned int m_numGlyphs;
    CharInfo *m_charinfo;  // character info, one per input character
    unsigned int m_numCharinfo;      // size of the array and number of input characters

    const LoadedFace *m_face;      // LoadedFace
    const Silf *m_silf;
    Position m_advance;       // whole segment advance
    Rect m_bbox;           // ink box of the segment
    int8 m_dir;
    std::vector<Features> m_feats;	// feature settings referenced by charinfos in this segment


#ifdef FIND_BROKEN_VIRTUALS
    virtual int8 dir() { return m_dir; }
#endif		//FIND_BROKEN_VIRTUALS

private:		//defensive on m_charinfo
    Segment(const Segment&);
    Segment& operator=(const Segment&);
};


#endif // SEGMENT_INCLUDE
