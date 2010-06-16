#include "FontFace.h"
#include <string.h>

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
    int numGlyphs = m_numGlyphs;
    if (locFlags)
    {
        if (lGloc < 4 * m_numGlyphs + 10)
            numGlyphs = (lGloc - 10) / 4;
    }
    else
    {
        if (lGloc < 2 * m_numGlyphs + 8)
            numGlyphs = (lGloc - 8) / 4;
    }

    for (int i = 0; i < numGlyphs; i++)
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
        g->readAttrs(pGlat, glocs, gloce, m_numAttrs);
    }
    return true;
}

bool FontFace::readGraphite()
{
    char *pSilf;
    size_t lSilf;
    if ((pSilf = (char *)getTable(ktiSilf, &lSilf)) == NULL) return false;
    uint32 version, compilerVersion;
    version = swap32(*(uint32 *)pSilf);
    if (version < 0x00020000) return false;
    if (version >= 0x00030000)
    {
        compilerVersion = swap32(((uint32 *)pSilf)[1]);
        pSilf += 4;
    }
    else
        m_numSilf = swap16(((uint16 *)pSilf)[2]);
    m_silfs = new Silf[m_numSilf];
    for (int i = 0; i < m_numSilf; i++)
    {
        uint32 offset = swap32(((uint32 *)pSilf)[2 + i]);
        uint32 next;
        if (i == m_numSilf - 1)
            next = lSilf;
        else
            next = swap32(((uint32 *)pSilf)[3 + i]);
        if (offset < 0 || offset > lSilf || next < 0 || next > lSilf) return false;
        if (!m_silfs[i].readGraphite((void *)((char *)pSilf + offset), next - offset, m_numGlyphs, version)) return false;
    }
    return true;
}

