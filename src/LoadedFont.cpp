#include "LoadedFont.h"

LoadedFont::LoadedFont(float ppm, const LoadedFace *face) :
    m_face(face),
    m_scale(ppm / face->upem())
{
    size_t nGlyphs=m_face->numGlyphs();
    m_advances = new float[nGlyphs];
    float *advp = m_advances;
    for (size_t i = 0; i < nGlyphs; i++)
    { *advp++ = INVALID_ADVANCE; }
}


/*virtual*/ LoadedFont::~LoadedFont()
{
    delete[] m_advances;
}


/*virtual*/ float LoadedFont::computeAdvance(unsigned short glyphid) const
{
    return m_face->getAdvance(glyphid, m_scale);
}



LoadedHintedFont::LoadedHintedFont(const IFont *font/*not NULL*/, const LoadedFace *face) :
    LoadedFont(font->ppm(), face), 
    m_font(font)
{
}


/*virtual*/ float LoadedHintedFont::computeAdvance(unsigned short glyphid) const
{
    return m_font->advance(glyphid);
}



