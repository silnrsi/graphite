#ifndef FONT_INCLUDE
#define FONT_INCLUDE

#include <limits>
#include "FontFace.h"
#include "TtfUtil.h"
#define NAN std::numeric_limits<float>::signaling_NaN()

class Font
{

public:
    Font(FontFace *face, float ppm);
    void *getTable(TableId name, size_t *len) { return m_face->getTable(name, len); }
    float advance(unsigned short glyphid) {
        if (m_advances[glyphid] == NAN)
            m_advances[glyphid] = m_face->getAdvance(glyphid, m_scale);
        return m_advances[glyphid];
    }

protected :
    float m_scale;      // scales from design units to ppm
    float *m_advances;  // One advance per glyph in pixels. Nan if not defined
    FontFace *m_face;   // FontFace to get the rest of the info from
};

#endif // FONT_INCLUDE
