#ifndef LOADEDFACE_INCLUDE
#define LOADEDFACE_INCLUDE

#include "GlyphFace.h"
#include "Silf.h"
#include "TtfUtil.h"
#include "Main.h"
#include "graphiteng/IFace.h"
#include "FeatureMap.h"

class Segment;
class Features;

// These are the actual tags, as distinct from the consecutive IDs in TtfUtil.h
#define tagCmap MAKE_TAG('c','m','a','p')
#define tagHead MAKE_TAG('h','e','a','d')
#define tagGlyf MAKE_TAG('g','l','y','f')
#define tagHdmx MAKE_TAG('h','d','m','x')
#define tagHhea MAKE_TAG('h','h','e','a')
#define tagHmtx MAKE_TAG('h','m','t','x')
#define tagLoca MAKE_TAG('l','o','c','a')
#define tagKern MAKE_TAG('k','e','r','n')
#define tagMaxp MAKE_TAG('m','a','x','p')
#define tagName MAKE_TAG('n','a','m','e')
#define tagOs2  MAKE_TAG('O','S','/','2')
#define tagPost MAKE_TAG('p','o','s','t')
#define tagFeat MAKE_TAG('F','e','a','t')
#define tagGlat MAKE_TAG('G','l','a','t')
#define tagGloc MAKE_TAG('G','l','o','c')
#define tagSilf MAKE_TAG('S','i','l','f')
#define tagSile MAKE_TAG('S','i','l','e')
#define tagSill MAKE_TAG('S','i','l','l')

class LoadedFace 	// an IFace loaded into an object
{
public:
    const void *getTable(unsigned int name, size_t *len) const { return m_face->getTable(name, len); }
    float advance(unsigned short id) const { return m_glyphs[id].advance().x; }
    const Silf *silf(int i) const { return ((i < m_numSilf) ? m_silfs + i : (const Silf *)NULL); }
    void runGraphite(Segment *seg, const Silf *silf) const;
    uint16 findPseudo(uint32 uid) const { return (m_numSilf) ? m_silfs[0].findPseudo(uid) : 0; }

public:
    LoadedFace(const IFace *face) : m_face(face), m_glyphs(NULL), m_silfs(NULL)  {}
    ~LoadedFace();
    const GlyphFace *glyph(unsigned short glyphid) const { return m_glyphs + glyphid; } // m_glyphidx[glyphid]; }
    float getAdvance(unsigned short glyphid, float scale) const { return advance(glyphid) * scale; }
    unsigned short upem() const { return m_upem; }
    unsigned short numGlyphs() const { return m_numGlyphs; }
    bool readGlyphs();
    bool readGraphite();
    bool readFeatures() { return m_features.readFont(m_face); }
    const Silf *chooseSilf(uint32 script) const;
    const FeatureMap& theFeatures() const { return m_features; }
    const FeatureRef *feature(uint8 index) const { return m_features.feature(index); }
    uint16 glyphAttr(uint16 gid, uint8 gattr) const { return (gattr < m_numAttrs && gid < m_numGlyphs) ? m_glyphs[gid].getAttr(gattr) : 0; }
    uint16 getGlyphMetric(uint16 gid, uint8 metric) const;
    
private:

    const IFace *m_face;                  // Where to get tables
    unsigned short m_numGlyphs;     // number of glyphs in the font
    uint16 m_ascent;
    uint16 m_descent;
    // unsigned short *m_glyphidx;     // index for each glyph id in the font
    // unsigned short m_readglyphs;    // how many glyphs have we in m_glyphs?
    // unsigned short m_capacity;      // how big is m_glyphs
    GlyphFace *m_glyphs;            // list of actual glyphs indexed by glyphidx, 1 base
    unsigned short m_upem;          // design units per em
    unsigned short m_numAttrs;      // number of glyph attributes per glyph
    unsigned short m_numSilf;       // number of silf subtables in the silf table
    Silf *m_silfs;                   // silf subtables.
    FeatureMap m_features;
    
  private:		//defensive on m_glyphs and m_silfs
    LoadedFace(const LoadedFace&);
    LoadedFace& operator=(const LoadedFace&);
};

#endif // LOADEDFACE_INCLUDE
