#ifndef ISEGMENT_INCLUDE
#define ISEGMENT_INCLUDE

#include "graphiteng/Types.h"
#include "graphiteng/ISlot.h"
#include "graphiteng/ITextSource.h"
#include <vector>

class IFontImpl;
class IFaceImpl;

class ISegment 
{
public:
    virtual int length() = 0;
    virtual Position advance() = 0;
    virtual ISlot & operator[] (int index) = 0;
    virtual const ISlot & operator[] (int index) const = 0;
};

extern ISegment *create_rangesegment(IFontImpl *font, IFaceImpl *face, ITextSource *txt);
extern void destroy_segment(ISegment *seg);

#endif // SEGMENT_INCLUDE
