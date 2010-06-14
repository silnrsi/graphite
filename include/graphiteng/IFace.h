#ifndef IFACE_INCLUDE
#define IFACE_INCLUDE

#include "graphiteng/Types.h"

class IFace
{
public:
    virtual void *getTable(unsigned int name, size_t *len) = 0;
};

#endif

