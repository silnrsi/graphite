#pragma once

#include "graphiteng/Types.h"

namespace org { namespace sil { namespace graphite { namespace v2 {

class GrFace;
class GrFont;

extern "C"
{
    GRNG_EXPORT GrFont* make_font(float ppm/*pixels per em*/, const GrFace *face);
                      //When finished with the GrFont, call destroy_font    
    typedef float (*advance_fn)(const void* appFontHandle, uint16 glyphid);     //amount to advance. positive is in the writing direction
    GRNG_EXPORT GrFont* make_font_with_advance_fn(float ppm/*pixels per em*/, const void* appFontHandle/*non-NULL*/, advance_fn advance, const GrFace *face);
                      //the appFontHandle must stay alive all the time when the GrFont is alive. When finished with the GrFont, call destroy_font    
    GRNG_EXPORT void destroy_font(GrFont *font);
}

}}}} // namespace
