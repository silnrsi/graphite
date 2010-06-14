#ifndef IFONT_INCLUDE
#define IFONT_INCLUDE

#include "graphiteng/Types.h"

class IFont
{

public:
    virtual float advance(uint16 glyphid) = 0;
    virtual float ppm() = 0;
};

#endif // FONT_INCLUDE
