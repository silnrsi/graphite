#ifndef IFONTIMPL_INCLUDE
#define IFONTIMPL_INCLUDE

#include "graphiteng/Types.h"

class IFont;
class IFaceImpl;

class IFontImpl
{
public:
    virtual float advance(uint16 glyphid) = 0;
};

extern IFontImpl *create_font(IFont *font, IFaceImpl *face, float ppm);
extern void destroy_fontface(IFontImpl *font);

#endif
