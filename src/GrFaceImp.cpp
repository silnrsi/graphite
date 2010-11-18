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
#include "GrFaceImp.h"
#include "VMScratch.h"
#include <string.h>
#include "GrSegmentImp.h"
#include "XmlTraceLog.h"

using namespace org::sil::graphite::v2;

GrFace::~GrFace()
{
    delete m_pGlyphFaceCache;
    delete[] m_silfs;
    m_pGlyphFaceCache = NULL;
    m_silfs = NULL;
    delete m_pFileFace;
    m_pFileFace = NULL;
}


bool GrFace::setGlyphCacheStrategy(EGlyphCacheStrategy requestedStrategy) const      //glyphs already loaded are unloaded
{
    GlyphFaceCache* pNewCache = GlyphFaceCache::makeCache(*m_pGlyphFaceCache, requestedStrategy);
    if (!pNewCache)
        return false;
    
    delete m_pGlyphFaceCache;
    m_pGlyphFaceCache = pNewCache;
    return true;
}


bool GrFace::readGlyphs(EGlyphCacheStrategy requestedStrategy)
{
    GlyphFaceCacheHeader hdr;
    if (!hdr.initialize(m_appFaceHandle, m_getTable)) return false;

    m_pGlyphFaceCache = GlyphFaceCache::makeCache(hdr, requestedStrategy);

    if (!m_pGlyphFaceCache) return false;
    m_upem = TtfUtil::DesignUnits(m_pGlyphFaceCache->m_pHead);
    // m_glyphidx = new unsigned short[m_numGlyphs];        // only need this if doing occasional glyph reads
    
    return true;
}

bool GrFace::readGraphite()
{
    char *pSilf;
    size_t lSilf;
    if ((pSilf = (char *)getTable(tagSilf, &lSilf)) == NULL) return false;
    uint32 version;
    uint32 compilerVersion = 0; // wasn't set before GTF version 3
    uint32 offset32Pos = 2;
    version = swap32(*(uint32 *)pSilf);
    if (version < 0x00020000) return false;
    if (version >= 0x00030000)
    {
        compilerVersion = swap32(((uint32 *)pSilf)[1]);
        m_numSilf = swap16(((uint16 *)pSilf)[4]);
        offset32Pos = 3;
    }
    else
        m_numSilf = swap16(((uint16 *)pSilf)[2]);

#ifndef DISABLE_TRACING
        if (XmlTraceLog::get().active())
        {
            XmlTraceLog::get().openElement(ElementSilf);
            XmlTraceLog::get().addAttribute(AttrMajor, version >> 16);
            XmlTraceLog::get().addAttribute(AttrMinor, version & 0xFFFF);
            XmlTraceLog::get().addAttribute(AttrCompilerMajor, compilerVersion >> 16);
            XmlTraceLog::get().addAttribute(AttrCompilerMinor, compilerVersion & 0xFFFF);
            XmlTraceLog::get().addAttribute(AttrNum, m_numSilf);
            if (m_numSilf == 0)
                XmlTraceLog::get().warning("No Silf subtables!");
        }
#endif

    m_silfs = new Silf[m_numSilf];
    for (int i = 0; i < m_numSilf; i++)
    {
        const uint32 offset = swap32(((uint32 *)pSilf)[offset32Pos + i]);
        uint32 next;
        if (i == m_numSilf - 1)
            next = lSilf;
        else
            next = swap32(((uint32 *)pSilf)[offset32Pos + 1 + i]);
        if (offset > lSilf || next > lSilf)
        {
#ifndef DISABLE_TRACING
            XmlTraceLog::get().error("Invalid table %d offset %d length %u", i, offset, lSilf);
            XmlTraceLog::get().closeElement(ElementSilf);
#endif
            return false;
        }
        if (!m_silfs[i].readGraphite((void *)((char *)pSilf + offset), next - offset, m_pGlyphFaceCache->m_nGlyphs, version))
        {
#ifndef DISABLE_TRACING
            if (XmlTraceLog::get().active())
            {
                XmlTraceLog::get().error("Error reading Graphite subtable %d", i);
                XmlTraceLog::get().closeElement(ElementSilfSub); // for convenience
                XmlTraceLog::get().closeElement(ElementSilf);
            }
#endif
            return false;
        }
    }

#ifndef DISABLE_TRACING
    XmlTraceLog::get().closeElement(ElementSilf);
#endif
    return true;
}

void GrFace::runGraphite(GrSegment *seg, const Silf *aSilf) const
{
    aSilf->runGraphite(seg, this);
}

const Silf *GrFace::chooseSilf(uint32 script) const
{
    if (m_numSilf == 0)
        return NULL;
    else if (m_numSilf == 1 || script == 0)
        return m_silfs;
    else // do more work here
        return m_silfs;
}

uint16 GrFace::getGlyphMetric(uint16 gid, uint8 metric) const
{
    switch ((enum metrics)metric)
    {
        case kgmetAscent : return m_ascent;
        case kgmetDescent : return m_descent;
        default: return m_pGlyphFaceCache->glyph(gid)->getMetric(metric);
    }
}

void GrFace::takeFileFace(FileFace* pFileFace/*takes ownership*/)
{
    if (m_pFileFace==pFileFace)
      return;
    
    delete m_pFileFace;
    m_pFileFace = pFileFace;
}

