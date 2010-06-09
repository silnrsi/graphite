#ifndef ISEGMENT_INCLUDE
#define ISEGMENT_INCLUDE

#include "graphiteng/Types.h"
#include "graphiteng/ISlot.h"
#include "graphiteng/ITextSource.h"

class ISegment 
{
public:
    virtual int numSlots() = 0;
    virtual Position advance() = 0;
//    virtual void append(const ISegment &other) = 0;
    virtual ISlot *slot(int index) = 0;
    virtual int first() = 0;;
    virtual int end() = 0;
};

extern ISegment *create_rangesegment(IFont *font, ITextSource *txt);

#endif // SEGMENT_INCLUDE
