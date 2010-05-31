#include "FontFace.h"

float FontFace::pixelAdvance(unsigned short id, float ppm)
{
    GlyphFace *g = newGlyph(id);
    return g->advance().x * ppm / m_upem;
}

