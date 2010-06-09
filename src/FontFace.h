#ifndef FONTFACE_INCLUDE
#define FONTFACE_INCLUDE

#include "GlyphFace.h"
#include "TtfUtil.h"

class FontFace
{
public:
    virtual void *getTable(TableId name, size_t *len) = 0;
    virtual float pixelAdvance(unsigned short id, float ppm);
    virtual void readGlyphs();

public:
    GlyphFace *glyph(unsigned short glyphid) { return m_glyphs + glyphid; } // m_glyphidx[glyphid]; }
    float getAdvance(unsigned short glyphid, float scale) { return pixelAdvance(glyphid, scale * m_upem); }
    unsigned short upem() { return m_upem; }
    unsigned short numGlyphs() { return m_numglyphs; }

protected:

    unsigned short m_numglyphs;     // number of glyphs in the font
    // unsigned short *m_glyphidx;     // index for each glyph id in the font
    // unsigned short m_readglyphs;    // how many glyphs have we in m_glyphs?
    // unsigned short m_capacity;      // how big is m_glyphs
    GlyphFace *m_glyphs;            // list of actual glyphs indexed by glyphidx, 1 base
    unsigned short m_upem;          // design units per em
};

#endif // FONTFACE_INCLUDE
