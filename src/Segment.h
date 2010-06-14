#ifndef SEGMENT_INCLUDE
#define SEGMENT_INCLUDE

#include "graphiteng/ISegment.h"
#include "graphiteng/ISlot.h"
#include "Slot.h"
#include "graphiteng/IFont.h"
#include "CharInfo.h"

class Segment : public ISegment
{
public:
    virtual int numSlots() { return m_numSlots; }
    virtual Position advance() { return m_advance; }
    virtual ISlot *slot(int index) { return m_slots + index; }
    virtual int first() { return m_first; }
    virtual int end() { return 0; }

    Segment(int numSlots, IFontImpl *font, IFaceImpl *face);
    Segment(const Segment &other);
    ~Segment();
    Slot *newSlot(int before, int after) {
        Slot *res;
        if (m_numSlots == m_maxSlots) growSlots(1);
        m_numSlots++;
        if (before == m_last) m_last = m_numSlots;
        res = m_slots + m_numSlots;
        res->init(m_numSlots);
        res->prev(before);
        res->next(after);
        return res;
    }
    void growSlots(int num);
    void initslots(int index, int cid, int gid);
    void positionSlots();
    void append(const Segment &other);

protected:
    Slot *m_slots;         // Array of slots - 1 since we don't use slots[0]
    int m_maxSlots;         // size of the array
    int m_numSlots;         // number of array entries used (index of last used slot)
    int m_last;             // index of last slot in the chain
    int m_first;            // indexc of first slot in the chain
    CharInfo *m_charinfo;  // character info, one per input character
    int m_numCharinfo;      // size of the array and number of input characters

    IFontImpl *m_font;      // Reference to font to get metrics from
    IFaceImpl *m_face;      // Fontface
    Position m_advance;       // whole segment advance
    Rect m_bbox;           // ink box of the segment
};

#endif // SEGMENT_INCLUDE
