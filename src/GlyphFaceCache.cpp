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
#include "GlyphFaceCache.h"
#include "graphite2/Font.h"
#include "GrFaceImp.h"     //for the tags



/*virtual*/ bool GlyphFaceCacheHeader::initialize(const void* appFaceHandle/*non-NULL*/, gr_get_table_fn getTable)    //return result indicates success. Do not use if failed.
{
    if ((m_pLoca = (*getTable)(appFaceHandle, tagLoca, &m_lLoca)) == NULL) return false;
    size_t lHead;
    if ((m_pHead = (*getTable)(appFaceHandle, tagHead, &lHead)) == NULL) return false;
    if ((m_pGlyf = (*getTable)(appFaceHandle, tagGlyf, &m_lGlyf)) == NULL) return false;
    if ((m_pHmtx = (*getTable)(appFaceHandle, tagHmtx, &m_lHmtx)) == NULL) return false;
    size_t lHHea;
    if ((m_pHHea = (*getTable)(appFaceHandle, tagHhea, &lHHea)) == NULL) return false;
    size_t lGlat;
    if ((m_pGlat = (*getTable)(appFaceHandle, tagGlat, &lGlat)) == NULL) return false;

    size_t lMaxp;
    const void* pMaxp = (*getTable)(appFaceHandle, tagMaxp, &lMaxp);
    if (pMaxp==NULL) return false;
    m_nGlyphsWithGraphics = (unsigned short)TtfUtil::GlyphCount(pMaxp);
    
    size_t lGloc;
    if ((m_pGloc = (*getTable)(appFaceHandle, tagGloc, &lGloc)) == NULL) return false;
    if (lGloc < 6) return false;
    int version = swap32(*((uint32 *)m_pGloc));
    if (version != 0x00010000) return false;

    m_numAttrs = swap16(((uint16 *)m_pGloc)[3]);

    unsigned short locFlags = swap16(((uint16 *)m_pGloc)[2]);
    if (locFlags&1)
    {
        m_locFlagsUse32Bit = true;
        m_nGlyphsWithAttributes = (unsigned short)((lGloc - 12) / 4);
    }
    else
    {
        m_locFlagsUse32Bit = false;
        m_nGlyphsWithAttributes = (unsigned short)((lGloc - 10) / 2);
    }
    
    if (m_nGlyphsWithAttributes>m_nGlyphsWithGraphics) 
        m_nGlyphs = m_nGlyphsWithAttributes;
    else
        m_nGlyphs = m_nGlyphsWithGraphics;

    return true;
}

/*static*/ GlyphFaceCache* GlyphFaceCache::makeCache(const GlyphFaceCacheHeader& hdr)
{
    return new(hdr) GlyphFaceCache(hdr);
}

GlyphFaceCache::GlyphFaceCache(const GlyphFaceCacheHeader& hdr)
:   GlyphFaceCacheHeader(hdr)
{
    unsigned int nGlyphs = numGlyphs();
    
    for (unsigned int i = 0; i < nGlyphs; i++)
    {
         *glyphPtrDirect(i) = NULL;
    }
}

/*virtual*/ GlyphFaceCache::~GlyphFaceCache()
{
//    delete[] m_glyphs;        //can't do this since not allocated by new[] and so does not know array size.
    unsigned int nGlyphs = numGlyphs();
    int deltaPointers = (*glyphPtrDirect(nGlyphs-1u) - *glyphPtrDirect(0u));
    if ((nGlyphs > 0u) && (deltaPointers == static_cast<int>(nGlyphs - 1)))
    {
        for (unsigned int i=0 ; i<nGlyphs; ++i)
        {
            GlyphFace *p = *glyphPtrDirect(i);
            assert (p);
            delete p;      //invokes d'tor. Does not release the memory.
        }
        free (*glyphPtrDirect(0));
    }
    else
    {
        for (unsigned int i=0 ; i<nGlyphs; ++i)
        {
            GlyphFace *p = *glyphPtrDirect(i);
            if (p)
            {
                delete p;      //invokes d'tor. Does not release the memory.
                free(p);
            }
        }
    }
}

void GlyphFaceCache::loadAllGlyphs()
{
    unsigned int nGlyphs = numGlyphs();
    GlyphFace * glyphs = gralloc<GlyphFace>(nGlyphs);
    for (unsigned short glyphid = 0; glyphid < nGlyphs; glyphid++)
    {
        GlyphFace **p = glyphPtrDirect(glyphid);
        *p = &(glyphs[glyphid]);
        new(*p) GlyphFace(*this, glyphid);
    }
}

/*virtual*/ const GlyphFace *GlyphFaceCache::glyph(unsigned short glyphid) const      //result may be changed by subsequent call with a different glyphid
{ 
    GlyphFace **p = glyphPtrDirect(glyphid);
    if (*p)
        return *p;

    *p = (GlyphFace*)malloc(sizeof(GlyphFace));
    new(*p) GlyphFace(*this, glyphid);
    return *p;
}
