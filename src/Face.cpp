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
#include "Face.h"
#include <string.h>
#include "Segment.h"
#include "CmapCache.h"
#include "NameTable.h"
#include "SegCacheStore.h"
#include "XmlTraceLog.h"
#include <graphite2/Segment.h>


using namespace graphite2;

Face::~Face()
{
    delete m_pGlyphFaceCache;
    delete m_cmapCache;
    delete[] m_silfs;
    m_pGlyphFaceCache = NULL;
    m_cmapCache = NULL;
    m_silfs = NULL;
    delete m_pFileFace;
    delete m_pNames;
    m_pFileFace = NULL;
}


bool Face::readGlyphs(unsigned int faceOptions)
{
    GlyphFaceCacheHeader hdr;
    if (!hdr.initialize(m_appFaceHandle, m_getTable)) return false;

    m_pGlyphFaceCache = GlyphFaceCache::makeCache(hdr);
    if (!m_pGlyphFaceCache) return false;
    if (faceOptions & gr_face_cacheCmap)
    {
        size_t length = 0;
        const void * table = getTable(tagCmap, &length);
        m_cmapCache = new CmapCache(table, length);
    }
    if (faceOptions & gr_face_preloadGlyphs)
    {
        m_pGlyphFaceCache->loadAllGlyphs();
    }
    m_upem = TtfUtil::DesignUnits(m_pGlyphFaceCache->m_pHead);
    // m_glyphidx = new unsigned short[m_numGlyphs];        // only need this if doing occasional glyph reads
    
    return true;
}

bool Face::readGraphite()
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
    bool havePasses = false;
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
        if (m_silfs[i].numPasses())
            havePasses = true;
    }

#ifndef DISABLE_TRACING
    XmlTraceLog::get().closeElement(ElementSilf);
#endif
    return havePasses;
}

void Face::runGraphite(Segment *seg, const Silf *aSilf) const
{
    aSilf->runGraphite(seg, 0, aSilf->numPasses());
}

const Silf *Face::chooseSilf(uint32 script) const
{
    if (m_numSilf == 0)
        return NULL;
    else if (m_numSilf == 1 || script == 0)
        return m_silfs;
    else // do more work here
        return m_silfs;
}

uint16 Face::getGlyphMetric(uint16 gid, uint8 metric) const
{
    switch ((enum metrics)metric)
    {
        case kgmetAscent : return m_ascent;
        case kgmetDescent : return m_descent;
        default: return m_pGlyphFaceCache->glyph(gid)->getMetric(metric);
    }
}

void Face::takeFileFace(FileFace* pFileFace/*takes ownership*/)
{
    if (m_pFileFace==pFileFace)
      return;
    
    delete m_pFileFace;
    m_pFileFace = pFileFace;
}

NameTable * Face::nameTable() const
{
    if (m_pNames) return m_pNames;
    size_t tableLength = 0;
    const void * table = getTable(tagName, &tableLength);
    if (table)
        m_pNames = new NameTable(table, tableLength);
    return m_pNames;
}

uint16 Face::languageForLocale(const char * locale) const
{
    nameTable();
    if (m_pNames)
        return m_pNames->getLanguageId(locale);
    return 0;
}


#ifndef DISABLE_FILE_FACE

FileFace::FileFace(const char *filename) :
    m_pHeader(NULL),
    m_pTableDir(NULL)
{
    if (!(m_pfile = fopen(filename, "rb"))) return;
    size_t lOffset, lSize;
    if (!TtfUtil::GetHeaderInfo(lOffset, lSize)) return;
    m_pHeader = (TtfUtil::Sfnt::OffsetSubTable*)gralloc<char>(lSize);
    if (fseek(m_pfile, lOffset, SEEK_SET)) return;
    if (fread(m_pHeader, 1, lSize, m_pfile) != lSize) return;
    if (!TtfUtil::CheckHeader(m_pHeader)) return;
    if (!TtfUtil::GetTableDirInfo(m_pHeader, lOffset, lSize)) return;
    m_pTableDir = (TtfUtil::Sfnt::OffsetSubTable::Entry*)gralloc<char>(lSize);
    if (fseek(m_pfile, lOffset, SEEK_SET)) return;
    if (fread(m_pTableDir, 1, lSize, m_pfile) != lSize) return;
}

FileFace::~FileFace()
{
    if (m_pTableDir)
        free(m_pTableDir);
    if (m_pHeader)
        free(m_pHeader);
    if (m_pfile)
        fclose(m_pfile);
    m_pTableDir = NULL;
    m_pfile = NULL;
    m_pHeader = NULL;
}

const void *FileFace::table_fn(const void* appFaceHandle, unsigned int name, size_t *len)
{
    const FileFace* ttfFaceHandle = (const FileFace*)appFaceHandle;
    TableCacheItem * res;
    switch (name)
    {
        case tagCmap:
            res = &ttfFaceHandle->m_tables[TtfUtil::ktiCmap];
            break;
//        case tagCvt:
//            res = &m_tables[TtfUtil::ktiCvt];
//            break;
//        case tagCryp:FileFace
//            res = &m_tables[TtfUtil::ktiCryp];
//            break;
        case tagHead:
            res = &ttfFaceHandle->m_tables[TtfUtil::ktiHead];
            break;
//        case tagFpgm:
//            res = &m_tables[TtfUtil::ktiFpgm];
//            break;
//        case tagGdir:
//            res = &m_tables[TtfUtil::ktiGdir];
//            break;
        case tagGlyf:
            res = &ttfFaceHandle->m_tables[TtfUtil::ktiGlyf];
            break;
        case tagHdmx:
            res = &ttfFaceHandle->m_tables[TtfUtil::ktiHdmx];
            break;
        case tagHhea:
            res = &ttfFaceHandle->m_tables[TtfUtil::ktiHhea];
            break;
        case tagHmtx:
            res = &ttfFaceHandle->m_tables[TtfUtil::ktiHmtx];
            break;
        case tagLoca:
            res = &ttfFaceHandle->m_tables[TtfUtil::ktiLoca];
            break;
        case tagKern:
            res = &ttfFaceHandle->m_tables[TtfUtil::ktiKern];
            break;
//        case tagLtsh:
//            res = &m_tables[TtfUtil::ktiLtsh];
//            break;
        case tagMaxp:
            res = &ttfFaceHandle->m_tables[TtfUtil::ktiMaxp];
            break;
        case tagName:
            res = &ttfFaceHandle->m_tables[TtfUtil::ktiName];
            break;
        case tagOs2:
            res = &ttfFaceHandle->m_tables[TtfUtil::ktiOs2];
            break;
        case tagPost:
            res = &ttfFaceHandle->m_tables[TtfUtil::ktiPost];
            break;
//        case tagPrep:
//            res = &m_tables[TtfUtil::ktiPrep];
//            break;
        case tagFeat:
            res = &ttfFaceHandle->m_tables[TtfUtil::ktiFeat];
            break;
        case tagGlat:
            res = &ttfFaceHandle->m_tables[TtfUtil::ktiGlat];
            break;
        case tagGloc:
            res = &ttfFaceHandle->m_tables[TtfUtil::ktiGloc];
            break;
        case tagSilf:
            res = &ttfFaceHandle->m_tables[TtfUtil::ktiSilf];
            break;
        case tagSile:
            res = &ttfFaceHandle->m_tables[TtfUtil::ktiSile];
            break;
        case tagSill:
            res = &ttfFaceHandle->m_tables[TtfUtil::ktiSill];
            break;
        default:
            res = NULL;
    }
    assert(res); // don't expect any other table types
    if (!res) return NULL;
    if (res->data() == NULL)
    {
        char *tptr;
        size_t tlen, lOffset;
        if (!TtfUtil::GetTableInfo(name, ttfFaceHandle->m_pHeader, ttfFaceHandle->m_pTableDir, lOffset, tlen)) return NULL;
        if (fseek(ttfFaceHandle->m_pfile, lOffset, SEEK_SET)) return NULL;
        tptr = gralloc<char>(tlen);
        if (fread(tptr, 1, tlen, ttfFaceHandle->m_pfile) != tlen) return NULL;
        res->set(tptr, tlen);
    }
    if (len) *len = res->size();
    return res->data();
}
#endif                  //!DISABLE_FILE_FACE
