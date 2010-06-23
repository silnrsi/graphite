#ifndef IFONT_INCLUDE
#define IFONT_INCLUDE

#include "graphiteng/Types.h"

class IFont
{

public:
    virtual float advance(uint16 glyphid) = 0;
    virtual float ppm() = 0;
};

class FontFace;

extern GRNG_EXPORT FontImpl *create_font(IFont *font, FontFace *face, float ppm);
extern GRNG_EXPORT void destroy_font(FontImpl *font);

class IFace;

extern GRNG_EXPORT FontFace *create_fontface(IFace *face);
extern GRNG_EXPORT void destroy_fontface(FontFace *face);

#ifndef DISABLE_FILE_FONT
extern GRNG_EXPORT FontFace *create_filefontface(const char * filePath);
#endif





#endif // FONT_INCLUDE
