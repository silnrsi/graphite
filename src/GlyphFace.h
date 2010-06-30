#pragma once

#include "XmlTraceLog.h"
#include "Main.h"
#include "Position.h"

enum metrics {
    kgmetLsb = 0, kgmetRsb,
    kgmetBbTop, kgmetBbBottom, kgmetBbLeft, kgmetBbRight,
    kgmetBbHeight, kgmetBbWidth,
    kgmetAdvWidth, kgmetAdvHeight,
    kgmetAscent, kgmetDescent
};

class Rect
{
public :
    Rect() {}
    Rect(const Position& botLeft, const Position& topRight): bl(botLeft), tr(topRight) {}
    Rect widen(const Rect& other) { return Rect(Position(bl.x > other.bl.x ? other.bl.x : bl.x, bl.y > other.bl.y ? other.bl.y : bl.y), Position(tr.x > other.tr.x ? tr.x : other.tr.x, tr.y > other.tr.y ? tr.y : other.tr.y)); }
    Rect operator + (const Position &a) const { return Rect(Position(bl.x + a.x, bl.y + a.y), Position(tr.x + a.x, tr.y + a.y)); }
    Rect operator * (float m) const { return Rect(Position(bl.x, bl.y) * m, Position(tr.x, tr.y) * m); }

    Position bl;
    Position tr;
};

class XmlTraceLog;

class GlyphFace
{
public:
    GlyphFace(Position pos=Position(), Rect box=Rect()) throw();
    ~GlyphFace() throw();

    const Position    & advance() const;
    void                advance(Position a);
    void    bbox(Rect a);
    const Rect &bbox() const { return m_bbox; }
    void    readAttrs(const void *pGlat, int start, int end, size_t num);
    uint16  getAttr(uint8 index) { 
        if (m_attrs)
            return m_attrs[index];
#ifdef ENABLE_DEEP_TRACING
        XmlTraceLog::get().warning("No attributes for glyph attr %d", index);
#endif
        return 0;
    }
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


