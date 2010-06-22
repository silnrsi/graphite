#ifndef IFONT_INCLUDE
#define IFONT_INCLUDE

#include "graphiteng/Types.h"

class IFont
{

public:
    virtual float advance(uint16 glyphid) = 0;
    virtual float ppm() = 0;
};

extern GRNG_EXPORT FontImpl *create_font(IFont *font, FontFace *face, float ppm);
extern GRNG_EXPORT void destroy_font(FontImpl *font);

#endif // FONT_INCLUDE
