#ifndef IFONT_INCLUDE
#define IFONT_INCLUDE

#include "graphiteng/Types.h"

class IFont
{

public:
    virtual float advance(uint16 glyphid) const = 0;		//amount to advance. positive is in the writing direction
    virtual float ppm() const = 0;				//pixels per em
private :
#ifdef FIND_BROKEN_VIRTUALS
    virtual void advance(uint16 glyphid) {}
    virtual void ppm() {}
#endif		//FIND_BROKEN_VIRTUALS
};




#endif // FONT_INCLUDE
