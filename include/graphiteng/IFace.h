#ifndef IFACE_INCLUDE
#define IFACE_INCLUDE

#include "graphiteng/Types.h"

class IFace
{
public:
    virtual void *getTable(unsigned int name, size_t *len) = 0;
};

extern GRNG_EXPORT FontFace *create_fontface(IFace *face);
extern GRNG_EXPORT void destroy_fontface(FontFace *face);

#ifndef DISABLE_FILE_FONT
extern GRNG_EXPORT FontFace *create_filefontface(const char * filePath);
#endif

#endif

