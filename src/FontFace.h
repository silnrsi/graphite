#ifndef FONTFACE_INCLUDE
#define FONTFACE_INCLUDE

#include "GlyphFace.h"
#include "TtfUtil.h"

class FontFace
{
public:
    void *getTable(TableId name);
    virtual float pixelAdvance(unsigned short id, float ppm);
    virtual GlyphFace *readGlyph(unsigned short glyphid);

public:
    GlyphFace *glyph(unsigned short glyphid) { return m_glyphs + m_glyphidx[glyphid]; }
    GlyphFace *newGlyph(unsigned short glyphid) {
        unsigned short idx = m_glyphidx[glyphid];
        if (!idx)
            return readGlyph(glyphid);
        else
            return m_glyphs + idx;
    }
    float getAdvance(unsigned short glyphid, float scale) { return pixelAdvance(glyphid, scale * m_upem); }

protected:
    unsigned short m_numglyphs;     // number of glyphs in the font
    unsigned short *m_glyphidx;     // index for each glyph id in the font
    GlyphFace *m_glyphs;            // list of actual glyphs indexed by glyphidx, 1 base
    unsigned short m_upem;          // design units per em
};

#endif // FONTFACE_INCLUDE
