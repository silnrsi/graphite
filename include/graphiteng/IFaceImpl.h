#ifndef IFACEIMPL_INCLUDE
#define IFACEIMPL_INCLUDE

#include "graphiteng/Types.h"

class IFace;

class IFaceImpl
{
public:
    virtual void *getTable(unsigned int name, size_t *len) = 0;
    virtual float getAdvance(uint16 glyphid, float scale) = 0;
    virtual uint16 upem() = 0;
    virtual uint16 numGlyphs() = 0;
};

extern IFaceImpl *create_fontface(IFace *face);
extern void destroy_fontface(IFaceImpl *face);

#endif
