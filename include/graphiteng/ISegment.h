#ifndef ISEGMENT_INCLUDE
#define ISEGMENT_INCLUDE

#include "graphiteng/Types.h"
#include "graphiteng/ISlot.h"
#include "graphiteng/ITextSource.h"
#include <vector>

class ISegment 
{
public:
    virtual int length() const = 0;
    virtual Position advance() const = 0;
    virtual ISlot & operator[] (int index) = 0;
    virtual const ISlot & operator[] (int index) const = 0;
    virtual void runGraphite() = 0;
    virtual void chooseSilf(uint32 script) = 0;
};

extern GRNG_EXPORT ISegment *create_rangesegment(FontImpl *font, FontFace *face, ITextSource *txt);
extern GRNG_EXPORT void destroy_segment(ISegment *seg);

#endif // SEGMENT_INCLUDE
