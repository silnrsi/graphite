#include "graphiteng/font.h"
#include "GrFont.h"

using namespace org::sil::graphite::v2;

extern "C" 
{
    GRNG_EXPORT GrFont* make_GrFont(float ppm/*pixels per em*/, const GrFace *face)
    {
        return new GrFont(ppm, face);
    }


    GRNG_EXPORT GrFont* make_GrFont_with_hints(float ppm/*pixels per em*/, const void* appFontHandle/*non-NULL*/, advance_fn advance, const GrFace *face)
    {                 //the appFontHandle must stay alive all the time when the GrFont is alive. When finished with the GrFont, call destroy_GrFont    
        return new GrHintedFont(ppm, appFontHandle, advance, face);
    }


    GRNG_EXPORT void destroy_GrFont(GrFont *font)
    {
        delete font;
    }
}




