#ifndef ISLOT_INCLUDE
#define ISLOT_INCLUDE

#include "graphiteng/Types.h"
#include "graphiteng/IFontImpl.h"

class ISlot
{
public:
    virtual unsigned short gid() = 0;
    virtual int next() = 0;
    virtual Position origin() = 0;
    virtual float advance(IFontImpl *font) = 0;
    virtual int before() = 0;
    virtual int after() = 0;
};

#endif // SLOT_INCLUDE

