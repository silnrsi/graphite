#include "GrFont.h"

using namespace org::sil::graphite::v2;

GrFont::GrFont(float ppm, const GrFace *face) :
    m_face(face),
    m_scale(ppm / face->upem())
{
    size_t nGlyphs=m_face->numGlyphs();
    m_advances = gralloc<float>(nGlyphs);
    float *advp = m_advances;
    for (size_t i = 0; i < nGlyphs; i++)
    { *advp++ = INVALID_ADVANCE; }
}


/*virtual*/ GrFont::~GrFont()
{
    free(m_advances);
}


/*virtual*/ float GrFont::computeAdvance(unsigned short glyphid) const
{
    return m_face->getAdvance(glyphid, m_scale);
}



GrHintedFont::GrHintedFont(float ppm/*pixels per em*/, const void* appFontHandle/*non-NULL*/, advance_fn advance2, const GrFace *face) :
    GrFont(ppm, face), 
    m_appFontHandle(appFontHandle),
    m_advance(advance2)
{
}


/*virtual*/ float GrHintedFont::computeAdvance(unsigned short glyphid) const
{
    return (*m_advance)(m_appFontHandle, glyphid);
}



