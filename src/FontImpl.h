#ifndef FONT_INCLUDE
#define FONT_INCLUDE

// #include <limits>
#include <cmath>
#include "graphiteng/IFont.h"
#include "FontFace.h"
// #define NAN std::numeric_limits<float>::signaling_NaN()

class FontImpl
{

public:
    FontImpl (IFont *font, FontFace *face, float ppm);
    float advance(unsigned short glyphid) {
        if (isnan(m_advances[glyphid]))
            m_advances[glyphid] = m_font ? m_font->advance(glyphid) : m_face->getAdvance(glyphid, m_scale);
        return m_advances[glyphid];
    }
    Position scale(Position p) { return Position(m_scale * p.x, m_scale * p.y); }
    float scale(float p) { return m_scale * p; }

protected :
    IFont *m_font;      // Application interface
    float m_scale;      // scales from design units to ppm
    float *m_advances;  // One advance per glyph in pixels. Nan if not defined
    FontFace *m_face;   // FontFace to get the rest of the info from
};

#endif // FONT_INCLUDE
