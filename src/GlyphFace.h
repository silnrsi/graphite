#ifndef GLYPHFACE_INCLUDE
#define GLYPHFACE_INCLUDE
#include "Main.h"
#include "XmlTraceLog.h"

class GlyphFace
{
public:
    GlyphFace(Position pos, Rect box) : m_bbox(box), m_advance(pos), m_attrs(NULL) { }
    ~GlyphFace() { if (m_attrs) delete[] m_attrs; }
    Position advance() { return m_advance; }
    void advance(Position a) { m_advance = a; }
    void bbox(Rect a) { m_bbox = a; }
    void readAttrs(const void *pGlat, int start, int end, unsigned short num) {
        m_attrs = new unsigned short[num];
        while (start < end)
        {
            int attr = ((char *)pGlat)[start];
            int count = ((char *)pGlat)[start + 1];
            for (int i = 0; i < count; i++)
            {
                m_attrs[attr + i] = swap16(((uint16 *)((char *)pGlat + start))[1 + i]);
#ifndef DISABLE_TRACING
                XmlTraceLog::get().openElement(ElementAttr);
                XmlTraceLog::get().addAttribute(AttrAttrId, attr + i);
                XmlTraceLog::get().addAttribute(AttrAttrVal, m_attrs[attr+i]);
                XmlTraceLog::get().closeElement(ElementAttr);
#endif
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
