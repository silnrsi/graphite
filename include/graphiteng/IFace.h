#ifndef IFACE_INCLUDE
#define IFACE_INCLUDE

#include "graphiteng/Types.h"

class IFace
{
public:
    virtual void *getTable(unsigned int name, size_t *len) = 0;
};

extern FontFace *create_fontface(IFace *face);
extern void destroy_fontface(FontFace *face);

#endif

