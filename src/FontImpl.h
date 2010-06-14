#ifndef FONT_INCLUDE
#define FONT_INCLUDE

// #include <limits>
#include <cmath>
#include "graphiteng/IFont.h"
#include "graphiteng/IFaceImpl.h"
#include "graphiteng/IFontImpl.h"
// #define NAN std::numeric_limits<float>::signaling_NaN()

class FontImpl : public IFontImpl
{

public:
    FontImpl (IFont *font, IFaceImpl *face, float ppm);
    virtual float advance(unsigned short glyphid) {
        if (isnan(m_advances[glyphid]))
            m_advances[glyphid] = m_font ? m_font->advance(glyphid) : m_face->getAdvance(glyphid, m_scale);
        return m_advances[glyphid];
    }

protected :
    IFont *m_font;      // Application interface
    float m_scale;      // scales from design units to ppm
    float *m_advances;  // One advance per glyph in pixels. Nan if not defined
    IFaceImpl *m_face;   // FontFace to get the rest of the info from
};

#endif // FONT_INCLUDE
