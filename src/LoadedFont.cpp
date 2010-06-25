#include "LoadedFont.h"

LoadedFont::LoadedFont(const IFont *font/*not NULL*/, const LoadedFace *face) :
    m_font(font),
    m_face(face),
    m_scale(font->ppm() / face->upem())
{
    initialize();
}


LoadedFont::LoadedFont(float ppm, const LoadedFace *face) :
    m_font(NULL),
    m_face(face),
    m_scale(ppm / face->upem())
{
    initialize();
}


void LoadedFont::initialize()
{
    size_t nGlyphs=m_face->numGlyphs();
    m_advances = new float[nGlyphs];
    float *advp = m_advances;
    for (size_t i = 0; i < nGlyphs; i++)
    { *advp++ = INVALID_ADVANCE; }
}


LoadedFont::~LoadedFont()
{
    delete[] m_advances;
}



