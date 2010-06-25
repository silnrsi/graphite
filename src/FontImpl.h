#ifndef FONT_INCLUDE
#define FONT_INCLUDE

//#include <limits>
//#include <cmath>
#include "graphiteng/IFont.h"
#include "LoadedFace.h"

class FontImpl
{
public:
    FontImpl(const IFont *font, const LoadedFace *face, float ppm);
    ~FontImpl();
    float advance(unsigned short glyphid) const {
        if (m_advances[glyphid] == INVALID_ADVANCE)
            m_advances[glyphid] = m_font ? m_font->advance(glyphid) : m_face->getAdvance(glyphid, m_scale);
        return m_advances[glyphid];
    }
    Position scale(const Position& p) const { return Position(m_scale * p.x, m_scale * p.y); }
    float scale(float p) const { return m_scale * p; }

private:
    const IFont *m_font;      // Application interface
    float m_scale;      // scales from design units to ppm
    float *m_advances;  // One advance per glyph in pixels. Nan if not defined
    const LoadedFace *m_face;   // LoadedFace to get the rest of the info from

    const static float INVALID_ADVANCE;
    
private:			//defensive on m_advances
    FontImpl(const FontImpl&);
    FontImpl& operator=(const FontImpl&);
};

#endif // FONT_INCLUDE
