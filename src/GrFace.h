#pragma once

#include "Main.h"
#include "GlyphFace.h"
#include "Silf.h"
#include "TtfUtil.h"
#include "Main.h"
#include "graphiteng/face.h"
#include "FeatureMap.h"
#include "GlyphFaceCache.h"

namespace org { namespace sil { namespace graphite { namespace v2 {

class Segment;
class Features;

// These are the actual tags, as distinct from the consecutive IDs in TtfUtil.h

#define tagGlat MAKE_TAG('G','l','a','t')
#define tagGloc MAKE_TAG('G','l','o','c')
#define tagGlyf MAKE_TAG('g','l','y','f')
#define tagHead MAKE_TAG('h','e','a','d')
#define tagHhea MAKE_TAG('h','h','e','a')
#define tagHmtx MAKE_TAG('h','m','t','x')
#define tagLoca MAKE_TAG('l','o','c','a')
#define tagMaxp MAKE_TAG('m','a','x','p')


#define tagCmap MAKE_TAG('c','m','a','p')
#define tagHdmx MAKE_TAG('h','d','m','x')
#define tagKern MAKE_TAG('k','e','r','n')
#define tagName MAKE_TAG('n','a','m','e')
#define tagOs2  MAKE_TAG('O','S','/','2')
#define tagPost MAKE_TAG('p','o','s','t')
#define tagFeat MAKE_TAG('F','e','a','t')
#define tagSilf MAKE_TAG('S','i','l','f')
#define tagSile MAKE_TAG('S','i','l','e')
#define tagSill MAKE_TAG('S','i','l','l')

class GrFace 	// an IFace loaded into an object
{
public:
    const void *getTable(unsigned int name, size_t *len) const { return (*m_getTable)(m_appFaceHandle, name, len); }
    float advance(unsigned short id) const { return m_pGlyphFaceCache->glyph(id)->theAdvance().x; }
    const Silf *silf(int i) const { return ((i < m_numSilf) ? m_silfs + i : (const Silf *)NULL); }
    void runGraphite(Segment *seg, const Silf *silf) const;
    uint16 findPseudo(uint32 uid) const { return (m_numSilf) ? m_silfs[0].findPseudo(uid) : 0; }

public:
    GrFace(const void* appFaceHandle/*non-NULL*/, get_table_fn getTable2) : m_appFaceHandle(appFaceHandle), m_getTable(getTable2), m_pGlyphFaceCache(NULL), m_silfs(NULL)  {}
    ~GrFace();
public:
    float getAdvance(unsigned short glyphid, float scale) const { return advance(glyphid) * scale; }
    const Rect &theBBoxTemporary(uint16 gid) const { return m_pGlyphFaceCache->glyph(gid)->theBBox(); }   //warning value may become invalid when another glyph is accessed
    unsigned short upem() const { return m_upem; }
    uint16 glyphAttr(uint16 gid, uint8 gattr) const { return m_pGlyphFaceCache->glyphAttr(gid, gattr); }

private:
    friend class GrFont;
    unsigned short numGlyphs() const { return m_pGlyphFaceCache->m_nGlyphs; }

public:
    bool setGlyphCacheStrategy(EGlyphCacheStrategy requestedStrategy) const;      //glyphs already loaded are unloaded
    bool readGlyphs(EGlyphCacheStrategy requestedStrategy);
    bool readGraphite();
    bool readFeatures() { return m_features.readFace(m_appFaceHandle/*non-NULL*/, m_getTable); }
    const Silf *chooseSilf(uint32 script) const;
    const FeatureMap& theFeatures() const { return m_features; }
    const FeatureRef *feature(uint8 index) const { return m_features.feature(index); }
    uint16 getGlyphMetric(uint16 gid, uint8 metric) const;

    const GlyphFaceCache* getGlyphFaceCache() const { return m_pGlyphFaceCache; }      //never NULL

    
    CLASS_NEW_DELETE
private:
    const void* m_appFaceHandle/*non-NULL*/;
    get_table_fn m_getTable;
    uint16 m_ascent;
    uint16 m_descent;
    // unsigned short *m_glyphidx;     // index for each glyph id in the font
    // unsigned short m_readglyphs;    // how many glyphs have we in m_glyphs?
    // unsigned short m_capacity;      // how big is m_glyphs
    mutable GlyphFaceCache* m_pGlyphFaceCache;      //owned - never NULL
    unsigned short m_upem;          // design units per em
    unsigned short m_numSilf;       // number of silf subtables in the silf table
    Silf *m_silfs;                   // silf subtables.
    FeatureMap m_features;
    
  private:		//defensive on m_pGlyphFaceCache and m_silfs
    GrFace(const GrFace&);
    GrFace& operator=(const GrFace&);
};

}}}} // namespace
