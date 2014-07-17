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

namespace graphite2 {

class Face;
class FeatureVal;
class GlyphFace;
class Segment;
class GlyphBox;

class GlyphCache
{
    class Loader;

    GlyphCache(const GlyphCache&);
    GlyphCache& operator=(const GlyphCache&);

public:
    GlyphCache(const Face & face, const uint32 face_options);
    ~GlyphCache();

    unsigned short  numGlyphs() const throw();
    unsigned short  numAttrs() const throw();
    unsigned short  unitsPerEm() const throw();

    const GlyphFace *glyph(unsigned short glyphid) const;      //result may be changed by subsequent call with a different glyphid
    const GlyphFace *glyphSafe(unsigned short glyphid) const;

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

class GlyphBox
{
public:
    GlyphBox(uint8 numsubs, ushort bitmap, Rect *slant) : _num(numsubs), _bitmap(bitmap), _slant(*slant) {}; 

    void addSubBox(int subindex, int boundary, Rect *val) { _subs[subindex * 2 + boundary] = *val; }
    Rect &subVal(int subindex, int boundary) { return _subs[subindex * 2 + boundary]; }

private:
    uint8   _num;
    ushort  _bitmap;
    Rect    _slant;
    Rect    _subs[0];
};

} // namespace graphite2
