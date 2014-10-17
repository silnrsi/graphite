/*  GRAPHITE2 LICENSING

    Copyright 2012, SIL International
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
    If not, write to the Free Software Foundation, 51 Franklin Street, 
    Suite 500, Boston, MA 02110-1335, USA or visit their web page on the 
    internet at http://www.fsf.org/licenses/lgpl.html.

Alternatively, the contents of this file may be used under the terms of the
Mozilla Public License (http://mozilla.org/MPL) or the GNU General Public
License, as published by the Free Software Foundation, either version 2
of the License or (at your option) any later version.
*/
#pragma once


#include "graphite2/Font.h"
#include "inc/Main.h"
#include "inc/Position.h"
#include "inc/GlyphFace.h"

namespace graphite2 {

class Face;
class FeatureVal;
class Segment;

class GlyphBox
{
public:
    GlyphBox(uint8 numsubs, unsigned short bitmap, Rect *slanted) : _num(numsubs), _bitmap(bitmap), _slant(*slanted) {}; 

    void addSubBox(int subindex, int boundary, Rect *val) { _subs[subindex * 2 + boundary] = *val; }
    Rect &subVal(int subindex, int boundary) { return _subs[subindex * 2 + boundary]; }
    const Rect &slant() const { return _slant; }
    uint8 num() const { return _num; }

private:
    uint8   _num;
    unsigned short  _bitmap;
    Rect    _slant;
    Rect    _subs[0];
};

class GlyphCache
{
    class Loader;

    GlyphCache(const GlyphCache&);
    GlyphCache& operator=(const GlyphCache&);

    static const Rect nullRect;

public:
    GlyphCache(const Face & face, const uint32 face_options);
    ~GlyphCache();

    unsigned short  numGlyphs() const throw();
    unsigned short  numAttrs() const throw();
    unsigned short  unitsPerEm() const throw();

    const GlyphFace *glyph(unsigned short glyphid) const;      //result may be changed by subsequent call with a different glyphid
    const GlyphFace *glyphSafe(unsigned short glyphid) const;
    float            getBoundingMetric(unsigned short glyphid, uint8 metric) const;
    uint8            numSubBounds(unsigned short glyphid) const;
    float            getSubBoundingMetric(unsigned short glyphid, uint8 subindex, uint8 metric) const;
    const Rect &     slant(unsigned short glyphid) const { return _boxes[glyphid] ? _boxes[glyphid]->slant() : nullRect; }

    CLASS_NEW_DELETE;
    
private:
    const Loader        * _glyph_loader;
    const GlyphFace *   * _glyphs;
    GlyphBox        *   * _boxes;
    unsigned short        _num_glyphs,
                          _num_attrs,
                          _upem;
};

inline
unsigned short GlyphCache::numGlyphs() const throw()
{
    return _num_glyphs;
}

inline
unsigned short GlyphCache::numAttrs() const throw()
{
    return _num_attrs;
}

inline
unsigned short  GlyphCache::unitsPerEm() const throw()
{
    return _upem;
}

inline
const GlyphFace *GlyphCache::glyphSafe(unsigned short glyphid) const
{
    return glyphid < _num_glyphs ? glyph(glyphid) : NULL;
}

inline
float GlyphCache::getBoundingMetric(unsigned short glyphid, uint8 metric) const
{
    if (glyphid >= _num_glyphs) return 0.;
    switch (metric) {
        case 0: return glyph(glyphid)->theBBox().bl.x;                          // x_min
        case 1: return glyph(glyphid)->theBBox().bl.y;                          // y_min
        case 2: return glyph(glyphid)->theBBox().tr.x;                          // x_max
        case 3: return glyph(glyphid)->theBBox().tr.y;                          // y_max
        case 4: return _boxes[glyphid] ? _boxes[glyphid]->slant().bl.x : 0.;    // sum_min
        case 5: return _boxes[glyphid] ? _boxes[glyphid]->slant().bl.y : 0.;    // diff_min
        case 6: return _boxes[glyphid] ? _boxes[glyphid]->slant().tr.x : 0.;    // sum_max
        case 7: return _boxes[glyphid] ? _boxes[glyphid]->slant().tr.y : 0.;    // diff_max
        default: return 0.;
    }
}

inline
float GlyphCache::getSubBoundingMetric(unsigned short glyphid, uint8 subindex, uint8 metric) const
{
    if (glyphid >= _num_glyphs) return 0.;
    GlyphBox *b = _boxes[glyphid];
    if (b == NULL || subindex >= b->num()) return 0;

    switch (metric) {
        case 0: return b->subVal(subindex, 0).bl.x;
        case 1: return b->subVal(subindex, 0).bl.y;
        case 2: return b->subVal(subindex, 0).tr.x;
        case 3: return b->subVal(subindex, 0).tr.y;
        case 4: return b->subVal(subindex, 1).bl.x;
        case 5: return b->subVal(subindex, 1).bl.y;
        case 6: return b->subVal(subindex, 1).tr.x;
        case 7: return b->subVal(subindex, 1).tr.y;
        default: return 0.;
    }
}

inline
uint8 GlyphCache::numSubBounds(unsigned short glyphid) const
{
    if (glyphid >= _num_glyphs) return 0;
    return _boxes[glyphid]->num();
}

} // namespace graphite2
