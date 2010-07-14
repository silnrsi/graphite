#include "GlyphFaceCache.h"
#include "graphiteng/IFace.h"
#include "TtfUtil.h"
#include "GrFace.h"     //for the tags

using namespace org::sil::graphite::v2;



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
    return true;
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


