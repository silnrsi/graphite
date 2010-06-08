#include "Font.h"
#include "FontFace.h"

Font::Font(FontFace *face, float ppm) :
    m_face(face), 
    m_scale(ppm / face->upem),
    m_advances(face->numGlyphs)
{
    float *advp = m_advances;
    for (int i = 0; i < face->numGlyphs; i++)
    { *advp++ = NAN; }
}

