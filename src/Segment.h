#ifndef SEGMENT_INCLUDE
#define SEGMENT_INCLUDE

#include <cassert>
#include <vector>

#include "graphiteng/ISegment.h"
#include "graphiteng/ISlot.h"
#include "Slot.h"
#include "CharInfo.h"
#include "Features.h"
#include "XmlTraceLog.h"

class Silf;
class LoadedFace;

class Segment : public ISegment
{
public:
    virtual int length() const { return m_numGlyphs; }
    virtual Position advance() const { return m_advance; }
    virtual Slot & operator[] (int index) {
        assert(index >= 0);
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
    virtual const Slot & operator[] (int index) const { return m_slots[index]; }
    virtual void runGraphite() { if (m_silf) m_face->runGraphite(this, m_silf); };
    virtual void chooseSilf(uint32 script) { m_silf = m_face->chooseSilf(script); }
    virtual CharInfo *charinfo(int index) { return index < m_numCharinfo ? m_charinfo + index : NULL; }

    Segment(int numSlots, const LoadedFace *face);
    Segment(const Segment &other);
    ~Segment();
    void appendSlot(int i, int cid, int gid, int fid);
    void positionSlots(FontImpl *font);
    void append(const Segment &other);
    void insertSlot(int index) { m_slots.insert(m_slots.begin() + index, Slot()); m_numGlyphs++; }
    void deleteSlot(int index) { m_slots.erase(m_slots.begin() + index); m_numGlyphs--; }
    uint16 getClassGlyph(uint16 cid, uint16 offset) { return const_cast<Silf *>(m_silf)->getClassGlyph(cid, offset); }
    uint16 findClassIndex(uint16 cid, uint16 gid) { return const_cast<Silf *>(m_silf)->findClassIndex(cid, gid); }
    int addFeatures(Features *feats) { m_feats.push_back(*feats); return m_feats.size() - 1; }
    uint16 getFeature(int index, uint8 findex) { return m_feats[index].getFeature(const_cast<LoadedFace *>(m_face)->feature(findex)); }
    int8 dir() { return m_dir; }
    void dir(int8 val) { m_dir = val; }
    uint16 glyphAttr(uint16 gid, uint8 gattr) { return const_cast<LoadedFace *>(m_face)->glyphAttr(gid, gattr); }
    uint16 getGlyphMetric(uint16 gid, uint8 metric) { return const_cast<LoadedFace *>(m_face)->getGlyphMetric(gid, metric); }

private:
    std::vector<Slot> m_slots;
    std::vector<uint16> m_user;
    int m_numGlyphs;
    CharInfo *m_charinfo;  // character info, one per input character
    int m_numCharinfo;      // size of the array and number of input characters

    const LoadedFace *m_face;      // LoadedFace
    const Silf *m_silf;
    Position m_advance;       // whole segment advance
    Rect m_bbox;           // ink box of the segment
    int8 m_dir;
    std::vector<Features> m_feats;	// feature settings referenced by charinfos in this segment
};

#ifndef DISABLE_TRACING
extern void logSegment(const ITextSource & textSrc, const ISegment & seg);
#endif

#endif // SEGMENT_INCLUDE
