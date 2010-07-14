#include "GrFace.h"
#include "VMScratch.h"
#include <string.h>
#include "Segment.h"
#include "XmlTraceLog.h"

using namespace org::sil::graphite::v2;

GrFace::~GrFace()
{
    delete m_pGlyphFaceCache;
    delete[] m_silfs;
    m_pGlyphFaceCache = NULL;
    m_silfs = NULL;
}


bool GlyphFaceCache::initialize(const IFace* iFace/*not NULL*/)
{
    if ((m_pLoca = iFace->getTable(tagLoca, &m_lLoca)) == NULL) return false;
    size_t lHead;
    if ((m_pHead = iFace->getTable(tagHead, &lHead)) == NULL) return false;
    size_t lGlyf;
    if ((m_pGlyf = iFace->getTable(tagGlyf, &lGlyf)) == NULL) return false;
    if ((m_pHmtx = iFace->getTable(tagHmtx, &m_lHmtx)) == NULL) return false;
    size_t lHHea;
    if ((m_pHHea = iFace->getTable(tagHhea, &lHHea)) == NULL) return false;
    size_t lGlat;
    if ((m_pGlat = iFace->getTable(tagGlat, &lGlat)) == NULL) return false;

    size_t lMaxp;
    const void* pMaxp = iFace->getTable(tagMaxp, &lMaxp);
    if (pMaxp==NULL) return false;
    m_nGlyphsWithGraphics = (unsigned short)TtfUtil::GlyphCount(pMaxp);
    
    size_t lGloc;
    if ((m_pGloc = iFace->getTable(tagGloc, &lGloc)) == NULL) return false;
    if (lGloc < 6) return false;
    int version = swap32(*((uint32 *)m_pGloc));
    if (version != 0x00010000) return false;

    m_numAttrs = swap16(((uint16 *)m_pGloc)[3]);

    unsigned short locFlags = swap16(((uint16 *)m_pGloc)[2]);
    if (locFlags&1)
    {
        m_locFlagsUse32Bit = true;
        m_nGlyphsWithAttributes = (unsigned short)((lGloc - 10) / 4);
    }
    else
    {
        m_locFlagsUse32Bit = false;
        m_nGlyphsWithAttributes = (unsigned short)((lGloc - 8) / 2);
    }
    
    if (m_nGlyphsWithAttributes>m_nGlyphsWithGraphics) 
        m_nGlyphs = m_nGlyphsWithAttributes;
    else
        m_nGlyphs = m_nGlyphsWithGraphics;

    m_glyphs2 = new GlyphFace [m_nGlyphs];
    if (!m_glyphs2)
        return false;
    
#ifndef DISABLE_TRACING
    if (XmlTraceLog::get().active())
    {
        XmlTraceLog::get().openElement(ElementGlyphs);
        XmlTraceLog::get().addAttribute(AttrNum, m_nGlyphs);
    }
#endif
    for (unsigned int i = 0; i < m_nGlyphs; i++)
    {
        setupGlyph(i);
    }
#ifndef DISABLE_TRACING
    XmlTraceLog::get().closeElement(ElementGlyphs);
#endif

}


void GlyphFaceCache::setupGlyph(unsigned short glyphid)
{
        Position pos(0, 0);
        Rect boundingBox(pos, pos);
        GlyphFace *g;
        int glocs, gloce;
        if (glyphid < m_nGlyphsWithGraphics)
        {
            int nLsb, xMin, yMin, xMax, yMax;
            unsigned int nAdvWid;
            size_t locidx = TtfUtil::LocaLookup(glyphid, m_pLoca, m_lLoca, m_pHead);
            void *pGlyph = TtfUtil::GlyfLookup(m_pGlyf, locidx);
            if (TtfUtil::HorMetrics(glyphid, m_pHmtx, m_lHmtx, m_pHHea, nLsb, nAdvWid))
                pos = Position(nAdvWid, 0);
            if (TtfUtil::GlyfBox(pGlyph, xMin, yMin, xMax, yMax))
                boundingBox = Rect(Position(xMin, yMin), Position(xMax - xMin, yMax - yMin));
        }
        g = new(m_glyphs2 + glyphid) GlyphFace(pos, boundingBox);
#ifndef DISABLE_TRACING
        if (XmlTraceLog::get().active())
        {
            XmlTraceLog::get().openElement(ElementGlyphFace);
            XmlTraceLog::get().addAttribute(AttrGlyphId, glyphid);
            XmlTraceLog::get().addAttribute(AttrAdvanceX, g->theAdvance().x);
            XmlTraceLog::get().addAttribute(AttrAdvanceY, g->theAdvance().y);
        }
#endif
        if (glyphid < m_nGlyphsWithAttributes)
        {
            if (m_locFlagsUse32Bit)
            {
                glocs = swap32(((uint32 *)m_pGloc)[2+glyphid]);
                gloce = swap32(((uint32 *)m_pGloc)[3+glyphid]);
            }
            else
            {
                glocs = swap16(((uint16 *)m_pGloc)[4+glyphid]);
                gloce = swap16(((uint16 *)m_pGloc)[5+glyphid]);
            }
            g->readAttrs(m_pGlat, glocs, gloce, m_numAttrs);
        }
#ifndef DISABLE_TRACING
        XmlTraceLog::get().closeElement(ElementGlyphFace);
#endif
}


bool GrFace::readGlyphs()
{
    m_pGlyphFaceCache = new GlyphFaceCache();
    if (!m_pGlyphFaceCache->initialize(m_face)) return false;
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

void GrFace::runGraphite(Segment *seg, const Silf *aSilf) const
{
    VMScratch vms;

    aSilf->runGraphite(seg, this, &vms);
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