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
    If not, write to the Free Software Foundation, 51 Franklin Street, 
    Suite 500, Boston, MA 02110-1335, USA or visit their web page on the 
    internet at http://www.fsf.org/licenses/lgpl.html.

Alternatively, the contents of this file may be used under the terms of the
Mozilla Public License (http://mozilla.org/MPL) or the GNU General Public
License, as published by the Free Software Foundation, either version 2
of the License or (at your option) any later version.
*/
#include <algorithm>

#include "graphite2/Font.h"

#include "inc/Main.h"
#include "inc/Face.h"     //for the tags
#include "inc/GlyphCache.h"
#include "inc/GlyphFace.h"
#include "inc/Endian.h"

using namespace graphite2;

namespace
{
    class glat_iterator : public std::iterator<std::input_iterator_tag, std::pair<sparse::key_type, sparse::mapped_type> >
    {
    public:
        glat_iterator(const void * glat=0) : _p(reinterpret_cast<const byte *>(glat)), _n(0) {}

        glat_iterator & operator ++ ()      { ++_v.first; --_n; _p += sizeof(uint16); if (_n == -1) { _p -= 2; _v.first = *_p++; _n = *_p++; } return *this; }
        glat_iterator   operator ++ (int)   { glat_iterator tmp(*this); operator++(); return tmp; }

        bool operator == (const glat_iterator & rhs) { return _p >= rhs._p || _p + _n*sizeof(uint16) > rhs._p; }
        bool operator != (const glat_iterator & rhs) { return !operator==(rhs); }

        value_type          operator * () const {
            if (_n==0) { _v.first = *_p++; _n = *_p++; }
            _v.second = be::peek<uint16>(_p);
            return _v;
        }
        const value_type *  operator ->() const { operator * (); return &_v; }

    protected:
        mutable const byte * _p;
        mutable value_type  _v;
        mutable int         _n;
    };

    class glat2_iterator : public glat_iterator
    {
    public:
        glat2_iterator(const void * glat) : glat_iterator(glat) {}

        glat_iterator & operator ++ ()      { ++_v.first; --_n; _p += sizeof(uint16); if (_n == -1) { _p -= sizeof(uint16)*2; _v.first = be::read<uint16>(_p); _n = be::read<uint16>(_p); } return *this; }
        glat_iterator   operator ++ (int)   { glat_iterator tmp(*this); operator++(); return tmp; }

        value_type          operator * () const {
            if (_n==0) { _v.first = be::read<uint16>(_p); _n = be::read<uint16>(_p); }
            _v.second = be::peek<uint16>(_p);
            return _v;
        }
        const value_type *  operator ->() const { operator * (); return &_v; }
    };
}


class GlyphCache::Loader
{
public:
    Loader(const Face & face, const bool dumb_font);    //return result indicates success. Do not use if failed.

    operator bool () const throw();
    unsigned short int units_per_em() const throw();
    unsigned short int num_glyphs() const throw();
    unsigned short int num_attrs() const throw();

    const GlyphFace * read_glyph(unsigned short gid, GlyphFace &) const throw();

    CLASS_NEW_DELETE;
private:
    Face::Table m_pHead,
                m_pHHea,
                m_pHmtx,
                m_pGlyf,
                m_pLoca,
                m_pGlat,
                m_pGloc;

    bool            m_locFlagsUse32Bit;
    unsigned short  m_nGlyphsWithGraphics,        //i.e. boundary box and advance
                    m_nGlyphsWithAttributes,
                    m_nAttrs;                    // number of glyph attributes per glyph
};



GlyphCache::GlyphCache(const Face & face, const bool dumb_font, bool preload)
: _glyph_loader(new Loader(face, dumb_font)),
  _glyphs(_glyph_loader && *_glyph_loader ? grzeroalloc<const GlyphFace *>(_glyph_loader->num_glyphs()) : 0),
  _num_glyphs(_glyphs ? _glyph_loader->num_glyphs() : 0),
  _num_attrs(_glyphs ? _glyph_loader->num_attrs() : 0),
  _upem(_glyphs ? _glyph_loader->units_per_em() : 0)
{
    if (!preload || !_glyph_loader || !_glyphs) return;

    GlyphFace * const glyphs = gralloc<GlyphFace>(_num_glyphs);
    if (!glyphs)
        return;

    // glyphs[0] has the same address as the glyphs array, thus assigning
    //  the &glyphs[0] to _glyphs[0] means _glyphs[0] points to the entire
    //  array.
    for (uint16 gid = 0; gid != _num_glyphs; ++gid)
        _glyphs[gid] = _glyph_loader->read_glyph(gid, glyphs[gid]);

    delete _glyph_loader;
    _glyph_loader = 0;
}


GlyphCache::~GlyphCache()
{
    const GlyphFace *  * g = _glyphs;
    if (_glyph_loader)
    {
    	for(unsigned short n = _num_glyphs; n; --n, ++g)
    		delete *g;
    }
    else
    {
    	for(unsigned short n = _num_glyphs; n; --n, ++g)
    		(*g)->~GlyphFace();
    	free(const_cast<GlyphFace *>(*_glyphs));
    }

    delete _glyph_loader;
}

const GlyphFace *GlyphCache::glyph(unsigned short glyphid) const      //result may be changed by subsequent call with a different glyphid
{ 
    const GlyphFace * & p = _glyphs[glyphid];
    if (p == 0 && _glyph_loader)
    {
        GlyphFace * g = new GlyphFace();
        if (g)  p = _glyph_loader->read_glyph(glyphid, *g);
    }
    return p;
}

uint16 GlyphCache::glyphAttr(uint16 gid, uint8 gattr) const
{
	if (gattr >= _num_attrs) return 0;

	const GlyphFace * p = glyphSafe(gid);

	return p ? p->attrs()[gattr] : 0;
}



GlyphCache::Loader::Loader(const Face & face, const bool dumb_font)
: m_pHead(face, Tag::head),
  m_pHHea(face, Tag::hhea),
  m_pHmtx(face, Tag::hmtx),
  m_pGlyf(face, Tag::glyf),
  m_pLoca(face, Tag::loca),
  m_nGlyphsWithGraphics(0),
  m_nGlyphsWithAttributes(0)
{
    if (!operator bool())
        return;

    const Face::Table maxp = Face::Table(face, Tag::maxp);
    if (!maxp) { m_pHead = Face::Table(); return; }

    m_nGlyphsWithGraphics = TtfUtil::GlyphCount(maxp);
    // This will fail if the number of glyphs is wildly out of range.
    if (TtfUtil::LocaLookup(m_nGlyphsWithGraphics-1, m_pLoca, m_pLoca.size(), m_pHead) == size_t(-1))
    {
        m_pHead = Face::Table();
        return;
    }

    if (!dumb_font)
    {
        if ((m_pGlat = Face::Table(face, Tag::Glat)) == NULL
            || (m_pGloc = Face::Table(face, Tag::Gloc)) == NULL
            || m_pGloc.size() < 6)
        {
            m_pHead = Face::Table();
            return;
        }
        const byte * p = m_pGloc;
        const int     version = be::read<uint32>(p);
        const uint16 locFlags = be::read<uint16>(p);
        m_nAttrs = be::read<uint16>(p);
        if (version != 0x00010000 || m_nAttrs > 0x1000) // is this hard limit appropriate?
        {
            m_pHead = Face::Table();
            return;
        }

        if (locFlags & 1)
        {
            m_locFlagsUse32Bit = true;
            m_nGlyphsWithAttributes = (m_pGloc.size() - 12) / 4;
        }
        else
        {
            m_locFlagsUse32Bit = false;
            m_nGlyphsWithAttributes = (m_pGloc.size() - 10) / 2;
        }
    }
}

inline
GlyphCache::Loader::operator bool () const throw()
{
    return bool(m_pHead);
}

inline
unsigned short int GlyphCache::Loader::units_per_em() const throw()
{
    return m_pHead ? TtfUtil::DesignUnits(m_pHead) : 0;
}

inline
unsigned short int GlyphCache::Loader::num_glyphs() const throw()
{
    return std::max(m_nGlyphsWithAttributes, m_nGlyphsWithGraphics);
}

inline
unsigned short int GlyphCache::Loader::num_attrs() const throw()
{
    return m_nAttrs;
}

const GlyphFace * GlyphCache::Loader::read_glyph(unsigned short glyphid, GlyphFace & glyph) const throw()
{
    Rect        bbox;
    Position    advance;
    size_t      glocs = 0, gloce = 0;


    if (glyphid < m_nGlyphsWithGraphics)
    {
        int nLsb, xMin, yMin, xMax, yMax;
        unsigned int nAdvWid;
        size_t locidx = TtfUtil::LocaLookup(glyphid, m_pLoca, m_pLoca.size(), m_pHead);
        void *pGlyph = TtfUtil::GlyfLookup(m_pGlyf, locidx, m_pGlyf.size());
        if (TtfUtil::HorMetrics(glyphid, m_pHmtx, m_pHmtx.size(), m_pHHea, nLsb, nAdvWid))
            advance = Position(static_cast<float>(nAdvWid), 0);

        if (pGlyph && TtfUtil::GlyfBox(pGlyph, xMin, yMin, xMax, yMax))
            bbox = Rect(Position(static_cast<float>(xMin), static_cast<float>(yMin)),
                Position(static_cast<float>(xMax), static_cast<float>(yMax)));
    }

    if (glyphid < m_nGlyphsWithAttributes)
    {
        const byte * gloc = m_pGloc;

        be::skip<uint32>(gloc);
        be::skip<uint16>(gloc,2);
        if (m_locFlagsUse32Bit)
        {
            be::skip<uint32>(gloc, glyphid);
            glocs = be::read<uint32>(gloc);
            gloce = be::peek<uint32>(gloc);
        }
        else
        {
            be::skip<uint16>(gloc, glyphid);
            glocs = be::read<uint16>(gloc);
            gloce = be::peek<uint16>(gloc);
        }
    }

    if (glocs < m_pGlat.size() && gloce <= m_pGlat.size())
    {
        const uint32 glat_version = be::peek<uint32>(m_pGlat);
        if (glat_version < 0x00020000)
        {
            if (gloce - glocs < 2*sizeof(byte)+sizeof(uint16)
                || gloce - glocs > m_nAttrs*(2*sizeof(byte)+sizeof(uint16)))
            {
                return 0;
            }

            new (&glyph) GlyphFace(bbox, advance, glat_iterator(m_pGlat + glocs), glat_iterator(m_pGlat + gloce));
        }
        else
        {
            if (gloce - glocs < 3*sizeof(uint16)
                || gloce - glocs > m_nAttrs*3*sizeof(uint16))
            {
                return 0;
            }

            new (&glyph) GlyphFace(bbox, advance, glat2_iterator(m_pGlat + glocs), glat2_iterator(m_pGlat + gloce));
        }

        if (glyph.attrs().size() > m_nAttrs)
        {
            glyph.~GlyphFace();
            new (&glyph) GlyphFace(bbox, advance, glat_iterator(), glat_iterator());
        }
    }

    return &glyph;
}
