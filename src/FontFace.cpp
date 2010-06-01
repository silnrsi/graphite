#include "FontFace.h"
#include <string.h>

float FontFace::pixelAdvance(unsigned short id, float ppm)
{
    GlyphFace *g = newGlyph(id);
    return g->advance().x * ppm / m_upem;
}

GlyphFace *FontFace::readGlyph(unsigned short gid)
{
    size_t lHead, lLoca, lGlyf, lHmtx, lHHea, lGloc, lGlat;
    void *pHead = getTable(ktiHead, &lHead);
    void *pHHea = getTable(ktiHhea, &lHHea);
    void *pLoca = getTable(ktiLoca, &lLoca);
    void *pGlyf = getTable(ktiGlyf, &lGlyf);
    void *pHmtx = getTable(ktiHmtx, &lHmtx);
    void *pGloc = getTable(ktiGloc, &lGloc);
    void *pGlat = getTable(ktiGlat, &lGlat);
    size_t locidx = TtfUtil::LocaLookup(gid, pLoca, lLoca, pHead);
    void *pGlyph = TtfUtil::GlyfLookup(pGlyf, locidx);
    int xMin, yMin, xMax, yMax;
    int nLsb;
    unsigned int nAdvWid;
    GlyphFace *glyph = addGlyph(gid);

    if (TtfUtil::HorMetrics(gid, pHmtx, lHmtx, pHHea, nLsb, nAdvWid))
        glyph->advance(Position(nAdvWid, 0));
    if (TtfUtil::GlyfBox(pGlyph, xMin, yMin, xMax, yMax))
        glyph->bbox(Rect(Position(xMin, yMin), Position(xMax - xMin, yMax - yMin)));
    return glyph;
}

GlyphFace *FontFace::addGlyph(unsigned short gid) 
{
    if (m_readglyphs >= m_capacity)
    {
        m_capacity *= 2;
        if (m_capacity > m_numglyphs) m_capacity = m_numglyphs;
        GlyphFace *newglyphs = static_cast<GlyphFace *>(operator new (m_capacity * sizeof(GlyphFace)));
        memcpy(newglyphs, m_glyphs, m_readglyphs * sizeof(GlyphFace));
        delete m_glyphs;
        m_glyphs = newglyphs;
    }
    m_readglyphs++;
    m_glyphidx[gid] = m_readglyphs;
    return m_glyphs + m_readglyphs;
}

