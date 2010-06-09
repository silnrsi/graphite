#ifndef IFONT_INCLUDE
#define IFONT_INCLUDE

#include "graphiteng/Types.h"

class IFont
{

public:
    virtual void *getTable(unsigned int name, size_t *len) = 0;
    virtual float advance(unsigned short glyphid) = 0;
};

#endif // FONT_INCLUDE
