#ifndef GLYPHFACE_INCLUDE
#define GLYPHFACE_INCLUDE
#include "Main.h"

class GlyphFace
{
public:
    GlyphFace(Position pos, Rect bbox) : m_bbox(bbox), m_advance(pos), m_attrs(NULL) { }
    ~GlyphFace() { if (m_attrs) delete[] m_attrs; }
    Position advance() { return m_advance; }
    void advance(Position a) { m_advance = a; }
    void bbox(Rect a) { m_bbox = a; }
    void readAttrs(void *pGlat, int start, int end, unsigned short num) {
        m_attrs = new unsigned short[num];
        while (start < end)
        {
            int attr = ((char *)pGlat)[start];
            int count = ((char *)pGlat)[start + 1];
            for (int i = 0; i < count; i++)
            {
                m_attrs[attr + i] = swap16(((uint16 *)pGlat)[start + 1 + i]);
            }
            start += 2 * (count + 1);
        }
    }

protected:
    Rect m_bbox;          // bounding box metrics in design units
    Position m_advance;   // Advance width and height in design units
    short *m_attribs;     // array of glyph attributes, fontface knows how many
    short *m_columns;     // array of fsm column values
    int m_gloc;           // glat offset
    unsigned short *m_attrs;
};

#endif // GLYPHFACE_INCLUDE
