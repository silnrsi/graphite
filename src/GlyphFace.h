#ifndef GLYPHFACE_INCLUDE
#define GLYPHFACE_INCLUDE
#include "Main.h"

enum metrics {
    kgmetLsb = 0, kgmetRsb,
    kgmetBbTop, kgmetBbBottom, kgmetBbLeft, kgmetBbRight,
    kgmetBbHeight, kgmetBbWidth,
    kgmetAdvWidth, kgmetAdvHeight,
    kgmetAscent, kgmetDescent
};
        
class GlyphFace
{
public:
    GlyphFace(Position pos=Position(), Rect box=Rect()) throw();
    ~GlyphFace() throw();

    const Position    & advance() const;
    void                advance(Position a);
    void    bbox(Rect a);
    void    readAttrs(const void *pGlat, int start, int end, size_t num);
    uint16  getAttr(uint8 index) { return m_attrs[index]; }
    uint16  getMetric(uint8 metric);

protected:
    Rect     m_bbox;        // bounding box metrics in design units
    Position m_advance;     // Advance width and height in design units
    short  * m_attribs;     // array of glyph attributes, fontface knows how many
    short  * m_columns;     // array of fsm column values
    int      m_gloc;        // glat offset
    unsigned short *m_attrs;
};


inline GlyphFace::GlyphFace(Position pos, Rect box) throw()
  : m_bbox(box), m_advance(pos), m_gloc(0),
    m_attribs(0), m_columns(0), m_attrs(0) {
}

inline GlyphFace::~GlyphFace() throw() { 
    delete [] m_attrs;
}

inline const Position & GlyphFace::advance() const { 
    return m_advance;
}

inline void GlyphFace::advance(Position a) { 
    m_advance = a;
}

inline void GlyphFace::bbox(Rect a) {
    m_bbox = a;
}


#endif // GLYPHFACE_INCLUDE
