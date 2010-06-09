#include "FontFace.h"
#include <string.h>
#include "graphiteng/Types.h"

float FontFace::pixelAdvance(unsigned short id, float ppm)
{
    // GlyphFace *g = newGlyph(id);
    GlyphFace *g = m_glyphs + id;
    return g->advance().x * ppm / m_upem;
}

void FontFace::readGlyphs()
{
    size_t lHead, lLoca, lGlyf, lHmtx, lHHea, lGloc, lGlat, lMaxp;
    void *pHead = getTable(ktiHead, &lHead);
    void *pHHea = getTable(ktiHhea, &lHHea);
    void *pLoca = getTable(ktiLoca, &lLoca);
    void *pGlyf = getTable(ktiGlyf, &lGlyf);
    void *pHmtx = getTable(ktiHmtx, &lHmtx);
    void *pGloc = getTable(ktiGloc, &lGloc);
    void *pGlat = getTable(ktiGlat, &lGlat);
    void *pMaxp = getTable(ktiMaxp, &lMaxp);
    m_numglyphs = TtfUtil::GlyphCount(pMaxp);
    m_upem = TtfUtil::DesignUnits(pHead);
    // m_glyphidx = new unsigned short[m_numglyphs];        // only need this if doing occasional glyph reads
    m_glyphs = static_cast<GlyphFace *>(operator new(m_numglyphs * sizeof(GlyphFace)));

    for (int i = 0; i < m_numglyphs; i++)
    {
        int nLsb, xMin, yMin, xMax, yMax;
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
        // m_glyphidx[i] = i;
        // read glyph attributes here
        // calculate FSM columns here
    }
}

