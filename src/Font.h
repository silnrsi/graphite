#ifndef FONT_INCLUDE
#define FONT_INCLUDE

// #include <limits>
#include <cmath>
#include "graphiteng/IFont.h"
#include "FontFace.h"
#include "TtfUtil.h"
// #define NAN std::numeric_limits<float>::signaling_NaN()

class Font : public IFont
{

public:
    virtual void *getTable(unsigned int name, size_t *len) { return m_face->getTable(name, len); }
    virtual float advance(unsigned short glyphid) {
        if (isnan(m_advances[glyphid]))
            m_advances[glyphid] = m_face->getAdvance(glyphid, m_scale);
        return m_advances[glyphid];
    }

    Font(FontFace *face, float ppm);

protected :
    float m_scale;      // scales from design units to ppm
    float *m_advances;  // One advance per glyph in pixels. Nan if not defined
    FontFace *m_face;   // FontFace to get the rest of the info from
};

#endif // FONT_INCLUDE
