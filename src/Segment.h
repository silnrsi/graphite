#ifndef SEGMENT_INCLUDE
#define SEGMENT_INCLUDE

#include <vector>

#include "graphiteng/ISegment.h"
#include "graphiteng/ISlot.h"
#include "Slot.h"
#include "CharInfo.h"
#include "FontFace.h"

class Segment : public ISegment
{
public:
    virtual int length() { return m_slots.size(); }
    virtual Position advance() { return m_advance; }
    virtual Slot & operator[] (int index) { return m_slots[index]; }
    virtual const Slot & operator[] (int index) const { return m_slots[index]; }
    virtual void runGraphite() { m_face->runGraphite(this, m_font); };

    Segment(int numSlots, FontImpl *font, FontFace *face);
    Segment(const Segment &other);
    ~Segment();
    void initslots(int index, int cid, int gid);
    void positionSlots();
    void append(const Segment &other);

protected:
    std::vector<Slot> m_slots;
    CharInfo *m_charinfo;  // character info, one per input character
    int m_numCharinfo;      // size of the array and number of input characters

    FontImpl *m_font;      // Reference to font to get metrics from
    FontFace *m_face;      // Fontface
    Position m_advance;       // whole segment advance
    Rect m_bbox;           // ink box of the segment
};

#endif // SEGMENT_INCLUDE
