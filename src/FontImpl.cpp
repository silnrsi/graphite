#include "FontImpl.h"
#include "graphiteng/IFaceImpl.h"
#include "graphiteng/IFont.h"

FontImpl::FontImpl(IFont *font, IFaceImpl *face, float ppm) :
    m_font(font),
    m_face(face),
    m_scale((font ? font->ppm() : ppm) / face->upem())
{
    m_advances = new float[face->numGlyphs()];
    float *advp = m_advances;
    for (int i = 0; i < face->numGlyphs(); i++)
    { *advp++ = NAN; }
}

