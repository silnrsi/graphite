#include "FontImpl.h"
#include "graphiteng/IFont.h"

const float FontImpl::INVALID_ADVANCE = -1e38f;

FontImpl::FontImpl(const IFont *font, LoadedFace *face, float ppm) :
    m_font(font),
    m_face(face),
    m_scale((font ? font->ppm() : ppm) / face->upem())
{
    size_t nGlyphs=face->numGlyphs();
    m_advances = new float[nGlyphs];
    float *advp = m_advances;
    for (size_t i = 0; i < nGlyphs; i++)
    { *advp++ = INVALID_ADVANCE; }
}

FontImpl::~FontImpl()
{
    delete[] m_advances;
}



