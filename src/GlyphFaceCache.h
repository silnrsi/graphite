/*  GRAPHITENG LICENSING

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
*/
#pragma once

#include "GlyphFace.h"
#include "graphite2/Font.h"


struct GrSegment;
struct GrFeatureVal;


class GlyphFaceCacheHeader
{
public:
    bool initialize(const void* appFaceHandle/*non-NULL*/, gr_get_table_fn getTable);    //return result indicates success. Do not use if failed.
    unsigned short numGlyphs() const { return m_nGlyphs; }
    unsigned short numAttrs() const { return m_numAttrs; }

private:
friend struct GrFace;
friend class GlyphFace;
    const void* m_pGloc;
    size_t m_lLoca;     const void* m_pLoca;
    const void* m_pHead;
    size_t m_lGlyf; const void* m_pGlyf;
    size_t m_lHmtx;     const void* m_pHmtx;
    const void* m_pHHea;
    const void* m_pGlat;

    unsigned short m_numAttrs;      // number of glyph attributes per glyph
    bool m_locFlagsUse32Bit;
    unsigned short m_nGlyphsWithGraphics;       //i.e. boundary box and advance
    unsigned short m_nGlyphsWithAttributes;
    unsigned short m_nGlyphs;                   // number of glyphs in the font. Max of the above 2.
};

class GlyphFaceCache : public GlyphFaceCacheHeader
{
public:
    static GlyphFaceCache* makeCache(const GlyphFaceCacheHeader& hdr /*, EGlyphCacheStrategy requested */);

    GlyphFaceCache(const GlyphFaceCacheHeader& hdr);
    ~GlyphFaceCache();

    const GlyphFace *glyphSafe(unsigned short glyphid) const { return glyphid<numGlyphs()?glyph(glyphid):NULL; }
    uint16 glyphAttr(uint16 gid, uint8 gattr) const { if (gattr>=numAttrs()) return 0; const GlyphFace*p=glyphSafe(gid); return p?p->getAttr(gattr):0; }

    void * operator new (size_t s, const GlyphFaceCacheHeader& hdr)
    {
        return malloc(s + sizeof(GlyphFace*)*hdr.numGlyphs());
    }
    // delete in case an exception is thrown in constructor
    void operator delete(void* p, const GlyphFaceCacheHeader& )
    {
        if (p) free(p);
    }

    const GlyphFace *glyph(unsigned short glyphid) const;      //result may be changed by subsequent call with a different glyphid
    void loadAllGlyphs();

    CLASS_NEW_DELETE
    
private:
    GlyphFace **glyphPtrDirect(unsigned short glyphid) const { return (GlyphFace **)((const char*)(this)+sizeof(GlyphFaceCache)+sizeof(GlyphFace*)*glyphid);}

private:      //defensive
    GlyphFaceCache(const GlyphFaceCache&);
    GlyphFaceCache& operator=(const GlyphFaceCache&);
};

