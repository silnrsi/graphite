#ifndef GLYPHFACE_INCLUDE
#define GLYPHFACE_INCLUDE
#include "Misc.h"

class GlyphFace
{
public:
    GlyphFace(Position pos, Rect bbox) : m_bbox(bbox), m_advance(pos) { }
    Position advance() { return m_advance; }
    void advance(Position a) { m_advance = a; }
    void bbox(Rect a) { m_bbox = a; }

protected:
    Rect m_bbox;          // bounding box metrics in design units
    Position m_advance;   // Advance width and height in design units
    short *m_attribs;     // array of glyph attributes, fontface knows how many
    short *m_columns;     // array of fsm column values
};

#endif // GLYPHFACE_INCLUDE
