#include "FontImpl.h"
#include "graphiteng/IFont.h"
#include "FontFace.h"

const float FontImpl::INVALID_ADVANCE = -1e38f;

FontImpl::FontImpl(IFont *font, FontFace *face, float ppm) :
    m_font(font),
    m_face(face),
    m_scale((font ? font->ppm() : ppm) / face->upem())
{
    m_advances = new float[face->numGlyphs()];
    float *advp = m_advances;
    for (int i = 0; i < face->numGlyphs(); i++)
    { *advp++ = INVALID_ADVANCE; }
}

