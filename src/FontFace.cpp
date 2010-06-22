#include "FontFace.h"
#include "VMScratch.h"
#include <string.h>
#include "Segment.h"
#include "XmlTraceLog.h"

float FontFace::advance(unsigned short id)
{
    // GlyphFace *g = newGlyph(id);
    GlyphFace *g = m_glyphs + id;
    return g->advance().x;
}

bool FontFace::readGlyphs()
{
    size_t lHead, lLoca, lGlyf, lHmtx, lHHea, lGloc, lGlat, lMaxp;
    void *pHead, *pHHea, *pLoca, *pGlyf, *pHmtx, *pGloc, *pGlat, *pMaxp;
    if ((pHead = getTable(ktiHead, &lHead)) == NULL) return false;
    if ((pHHea = getTable(ktiHhea, &lHHea)) == NULL) return false;
    if ((pLoca = getTable(ktiLoca, &lLoca)) == NULL) return false;
    if ((pGlyf = getTable(ktiGlyf, &lGlyf)) == NULL) return false;
    if ((pHmtx = getTable(ktiHmtx, &lHmtx)) == NULL) return false;
    if ((pGloc = getTable(ktiGloc, &lGloc)) == NULL) return false;
    if ((pGlat = getTable(ktiGlat, &lGlat)) == NULL) return false;
    if ((pMaxp = getTable(ktiMaxp, &lMaxp)) == NULL) return false;
    m_numGlyphs = TtfUtil::GlyphCount(pMaxp);
    m_upem = TtfUtil::DesignUnits(pHead);
    // m_glyphidx = new unsigned short[m_numGlyphs];        // only need this if doing occasional glyph reads
    m_glyphs = static_cast<GlyphFace *>(operator new(m_numGlyphs * sizeof(GlyphFace)));

    int version = swap32(*((uint32 *)pGloc));
    if (version != 0x00010000) return false;
    if (lGloc < 6) return false;
    unsigned short locFlags = swap16(((uint16 *)pGloc)[2]);
    m_numAttrs = swap16(((uint16 *)pGloc)[3]);
    int nGlyphs = m_numGlyphs;
    if (locFlags)
    {
        if (lGloc < 4 * m_numGlyphs + 10)
            nGlyphs = (lGloc - 10) / 4;
    }
    else
    {
        if (lGloc < 2 * m_numGlyphs + 8)
            nGlyphs = (lGloc - 8) / 4;
    }
    XmlTraceLog::get().openElement(ElementGlyphs);
    XmlTraceLog::get().addAttribute(AttrNum, nGlyphs);
    for (int i = 0; i < nGlyphs; i++)
    {
        int nLsb, xMin, yMin, xMax, yMax, glocs, gloce;
        unsigned int nAdvWid;
        Position pos(0, 0);
        Rect bbox(pos, pos);
        GlyphFace *g;
        size_t locidx = TtfUtil::LocaLookup(i, pLoca, lLoca, pHead);
        void *pGlyph = TtfUtil::GlyfLookup(pGlyf, locidx);
        if (TtfUtil::HorMetrics(i, pHmtx, lHmtx, pHHea, nLsb, nAdvWid))
            pos = Position(nAdvWid, 0);
        if (TtfUtil::GlyfBox(pGlyph, xMin, yMin, xMax, yMax))
            bbox = Rect(Position(xMin, yMin), Position(xMax - xMin, yMax - yMin));
        g = new(m_glyphs + i) GlyphFace(pos, bbox);
        if (locFlags & 1)
        {
            glocs = swap32(((uint32 *)pGloc)[2+i]);
            gloce = swap32(((uint32 *)pGloc)[3+i]);
        }
        else
        {
            glocs = swap16(((uint16 *)pGloc)[4+i]);
            gloce = swap16(((uint16 *)pGloc)[5+i]);
        }
        XmlTraceLog::get().openElement(ElementGlyphFace);
        XmlTraceLog::get().addAttribute(AttrGlyphId, i);
        XmlTraceLog::get().addAttribute(AttrAdvanceX, g->advance().x);
        XmlTraceLog::get().addAttribute(AttrAdvanceY, g->advance().y);
        g->readAttrs(pGlat, glocs, gloce, m_numAttrs);
        XmlTraceLog::get().closeElement(ElementGlyphFace);
    }
    XmlTraceLog::get().closeElement(ElementGlyphs);
    return true;
}

bool FontFace::readGraphite()
{
    char *pSilf;
    size_t lSilf;
    if ((pSilf = (char *)getTable(ktiSilf, &lSilf)) == NULL) return false;
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

    XmlTraceLog::get().openElement(ElementSilf);
    XmlTraceLog::get().addAttribute(AttrMajor, version >> 16);
    XmlTraceLog::get().addAttribute(AttrMinor, version & 0xFFFF);
    XmlTraceLog::get().addAttribute(AttrCompilerMajor, compilerVersion >> 16);
    XmlTraceLog::get().addAttribute(AttrCompilerMinor, compilerVersion & 0xFFFF);
    XmlTraceLog::get().addAttribute(AttrNum, m_numSilf);
    if (m_numSilf == 0)
        XmlTraceLog::get().warning("No Silf subtables!");
    m_silfs = new Silf[m_numSilf];
    for (int i = 0; i < m_numSilf; i++)
    {
        uint32 offset = swap32(((uint32 *)pSilf)[offset32Pos + i]);
        uint32 next;
        if (i == m_numSilf - 1)
            next = lSilf;
        else
            next = swap32(((uint32 *)pSilf)[offset32Pos + 1 + i]);
        if (offset < 0 || offset > lSilf || next < 0 || next > lSilf)
        {
            XmlTraceLog::get().error("Invalid table %d offset %d length %u", i, offset, lSilf);
            XmlTraceLog::get().closeElement(ElementSilf);
            return false;
        }
        if (!m_silfs[i].readGraphite((void *)((char *)pSilf + offset), next - offset, m_numGlyphs, version))
        {
            XmlTraceLog::get().error("Error reading Graphite subtable %d", i);
            XmlTraceLog::get().closeElement(ElementSilfSub); // for convenience
            XmlTraceLog::get().closeElement(ElementSilf);
            return false;
        }
    }
    XmlTraceLog::get().closeElement(ElementSilf);
    return true;
}

void FontFace::runGraphite(Segment *seg)
{
    VMScratch vms;

    if (m_numSilf > 0)
        m_silfs[0].runGraphite(seg, this, &vms);
}
