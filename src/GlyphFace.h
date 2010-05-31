#ifndef GLYPHFACE_INCLUDE
#define GLYPHFACE_INCLUDE
#include "Misc.h"

class GlyphFace
{
public:
    Position advance() { return m_advance; }

protected:
    Rect m_bbox;          // bounding box metrics in design units
    Position m_advance;   // Advance width and height in design units
    short *m_attribs;     // array of glyph attributes, fontface knows how many
    short *m_columns;     // array of fsm column values
};

#endif // GLYPHFACE_INCLUDE
