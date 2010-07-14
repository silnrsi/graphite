#pragma once

#include "GlyphFace.h"

namespace org { namespace sil { namespace graphite { namespace v2 {

class Segment;
class Features;
class IFace;


class GlyphFaceCache
{
public:
    bool initialize(const IFace* iFace/*not NULL*/);
    const GlyphFace *glyph(unsigned short glyphid) const { return m_glyphs2 + glyphid; }
    const GlyphFace *glyphSafe(unsigned short glyphid) const { return glyphid<m_nGlyphs?m_glyphs2+glyphid:NULL; }
    uint16 glyphAttr(uint16 gid, uint8 gattr) const { if (gattr>=m_numAttrs) return 0; const GlyphFace*p=glyphSafe(gid); return p?p->getAttr(gattr):0; }

    CLASS_NEW_DELETE
        
protected:
    void setupGlyph(unsigned short glyphid);

private:
friend class GrFace;
    const void* m_pGloc;
    size_t m_lLoca;     const void* m_pLoca;
    const void* m_pHead;
    const void* m_pGlyf;
    size_t m_lHmtx;     const void* m_pHmtx;
    const void* m_pHHea;
    const void* m_pGlat;

    unsigned short m_numAttrs;      // number of glyph attributes per glyph
    bool m_locFlagsUse32Bit;
    unsigned short m_nGlyphsWithGraphics;       //i.e. boundary box and advance
    unsigned short m_nGlyphsWithAttributes;
    unsigned short m_nGlyphs;                   // number of glyphs in the font. MAx of the above 2.
    GlyphFace *m_glyphs2;                       // list of actual glyphs indexed by glyphidx.
};



}}}} // namespace
