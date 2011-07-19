/*  GRAPHITE2 LICENSING

    Copyright 2010, SIL International
    All rights reserved.

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published
    by the Free Software Foundation; either version 2.1 of License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should also have received a copy of the GNU Lesser General Public
    License along with this library in the file named "LICENSE".
    If not, write to the Free Software Foundation, Inc., 59 Temple Place, 
    Suite 330, Boston, MA 02111-1307, USA or visit their web page on the 
    internet at http://www.fsf.org/licenses/lgpl.html.

Alternatively, the contents of this file may be used under the terms of the
Mozilla Public License (http://mozilla.org/MPL) or the GNU General Public
License, as published by the Free Software Foundation, either version 2
of the License or (at your option) any later version.
*/
#pragma once

#include "Main.h"
#include "XmlTraceLog.h"
#include "Main.h"
#include "Position.h"

namespace graphite2 {

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
class GlyphFaceCacheHeader;

class GlyphFace
{
private:
friend class GlyphFaceCache;
    GlyphFace(const GlyphFaceCacheHeader& hdr, unsigned short glyphid);
    ~GlyphFace() throw();
    void * operator new (size_t /*s*/, GlyphFace * p)
    {
        return p;
    }
    // delete in case an exception is thrown in constructor
    void operator delete(void*, GlyphFace*) {}
    void operator delete (void *) {}

public:

    const Position    & theAdvance() const;
//    void                setAdvance(const Position& a);
//    void    setBBox(const Rect& a);
    const Rect &theBBox() const { return m_bbox; }
    uint16  getAttr(uint8 index) const { 
        if (m_attrs)
            return m_attrs[index];
#ifdef ENABLE_DEEP_TRACING
        XmlTraceLog::get().warning("No attributes for glyph attr %d", index);
#endif
        return 0;
    }
    uint16  getMetric(uint8 metric) const;

private:
    void    readAttrs(const void *pGlat, int start, int end, size_t num, uint32 format);       //only called from constructor

private:
    Rect     m_bbox;        // bounding box metrics in design units
    Position m_advance;     // Advance width and height in design units
//    short  * m_attribs;     // array of glyph attributes, fontface knows how many
//    short  * m_columns;     // array of fsm column values
//    int      m_gloc;        // glat offset
    unsigned short *m_attrs;
};


#if 0
inline GlyphFace::GlyphFace(Position pos, Rect box) throw()
  : m_bbox(box), m_advance(pos), m_gloc(0),
//    m_attribs(0), m_columns(0), 
    m_attrs(0) {
}
#endif

inline GlyphFace::~GlyphFace() throw() {
    if (m_attrs) free(m_attrs);
}

inline const Position & GlyphFace::theAdvance() const { 
    return m_advance;
}

#if 0
inline void GlyphFace::setAdvance(const Position& a) { 
    m_advance = a;
}

inline void GlyphFace::setBBox(const Rect& a) {
    m_bbox = a;
}
#endif

} // namespace graphite2
