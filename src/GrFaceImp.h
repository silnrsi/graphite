/*  GRAPHITENG LICENSING

    Copyright 2010, SIL International
    All rights reserved.

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published
    by the Free Software Foundation; either version 2.1 of License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should also have received a copy of the GNU Lesser General Public
    License along with this library in the file named "LICENSE".
    If not, write to the Free Software Foundation, Inc., 59 Temple Place, 
    Suite 330, Boston, MA 02111-1307, USA or visit their web page on the 
    internet at http://www.fsf.org/licenses/lgpl.html.
*/
#pragma once

#include "Main.h"
#include "GlyphFace.h"
#include "Silf.h"
#include "TtfUtil.h"
#include "Main.h"
#include "graphite2/Font.h"
#include "FeatureMap.h"
#include "GlyphFaceCache.h"

#ifndef DISABLE_FILE_FACE
#include <cstdio>
#include <cassert>
#include "TtfTypes.h"
#endif      //!DISABLE_FILE_FACE
namespace org { namespace sil { namespace graphite { namespace v2 {

struct GrSegment;
struct GrFeatureVal;
class NameTable;
class CmapCache;

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

#ifndef DISABLE_FILE_FACE
class TableCacheItem
{
public:
    TableCacheItem(char * theData, size_t theSize) : m_data(theData), m_size(theSize) {}
    TableCacheItem() : m_data(0), m_size(0) {}
    ~TableCacheItem()
    {
        if (m_size) free(m_data);
    }
    void set(char * theData, size_t theSize) { m_data = theData; m_size = theSize; }
    const void * data() const { return m_data; }
    size_t size() const { return m_size; }
private:
    char * m_data;
    size_t m_size;
};
#endif      //!DISABLE_FILE_FACE




class FileFace
{
#ifndef DISABLE_FILE_FACE
public:
    FileFace(const char *filename);
    ~FileFace();
//    virtual const void *getTable(unsigned int name, size_t *len) const;
    bool isValid() const { return m_pfile && m_pHeader && m_pTableDir; }

    CLASS_NEW_DELETE
public:     //for local convenience
    FILE* m_pfile;
    mutable TableCacheItem m_tables[TtfUtil::ktiLast];
    TtfUtil::Sfnt::OffsetSubTable* m_pHeader;
    TtfUtil::Sfnt::OffsetSubTable::Entry* m_pTableDir;       //[] number of elements is determined by m_pHeader->num_tables
#endif      //!DISABLE_FILE_FACE
   
private:        //defensive
    FileFace(const FileFace&);
    FileFace& operator=(const FileFace&);
};

const void *FileFace_table_fn(const void* appFaceHandle, unsigned int name, size_t *len);

struct GrFace
{
public:
    const void *getTable(unsigned int name, size_t *len) const { return (*m_getTable)(m_appFaceHandle, name, len); }
    float advance(unsigned short id) const { return m_pGlyphFaceCache->glyph(id)->theAdvance().x; }
    const Silf *silf(int i) const { return ((i < m_numSilf) ? m_silfs + i : (const Silf *)NULL); }
    virtual void runGraphite(GrSegment *seg, const Silf *silf) const;
    uint16 findPseudo(uint32 uid) const { return (m_numSilf) ? m_silfs[0].findPseudo(uid) : 0; }

public:
    GrFace(const void* appFaceHandle/*non-NULL*/, gr_get_table_fn getTable2) : 
        m_appFaceHandle(appFaceHandle), m_getTable(getTable2), m_pGlyphFaceCache(NULL),
        m_cmapCache(NULL), m_numSilf(0), m_silfs(NULL), m_pFileFace(NULL),
        m_pNames(NULL) {}
    virtual ~GrFace();
public:
    float getAdvance(unsigned short glyphid, float scale) const { return advance(glyphid) * scale; }
    const Rect &theBBoxTemporary(uint16 gid) const { return m_pGlyphFaceCache->glyph(gid)->theBBox(); }   //warning value may become invalid when another glyph is accessed
    unsigned short upem() const { return m_upem; }
    uint16 glyphAttr(uint16 gid, uint8 gattr) const { return m_pGlyphFaceCache->glyphAttr(gid, gattr); }

private:
    friend struct GrFont;
    unsigned short numGlyphs() const { return m_pGlyphFaceCache->m_nGlyphs; }

public:
    bool readGlyphs(unsigned int faceOptions);
    bool readGraphite();
    bool readFeatures() { return m_Sill.readFace(m_appFaceHandle/*non-NULL*/, m_getTable, this); }
    const Silf *chooseSilf(uint32 script) const;
    const SillMap& theSill() const { return m_Sill; }
    uint16 numFeatures() const { return m_Sill.m_FeatureMap.numFeats(); }
    const GrFeatureRef *featureById(uint32 id) const { return m_Sill.m_FeatureMap.findFeatureRef(id); }
    const GrFeatureRef *feature(uint16 index) const { return m_Sill.m_FeatureMap.feature(index); }
    uint16 getGlyphMetric(uint16 gid, uint8 metric) const;

    const GlyphFaceCache* getGlyphFaceCache() const { return m_pGlyphFaceCache; }      //never NULL
    void takeFileFace(FileFace* pFileFace/*takes ownership*/);
    CmapCache * getCmapCache() const { return m_cmapCache; };
    NameTable * nameTable() const;
    uint16 languageForLocale(const char * locale) const;

    CLASS_NEW_DELETE
private:
    const void* m_appFaceHandle/*non-NULL*/;
    gr_get_table_fn m_getTable;
    uint16 m_ascent;
    uint16 m_descent;
    // unsigned short *m_glyphidx;     // index for each glyph id in the font
    // unsigned short m_readglyphs;    // how many glyphs have we in m_glyphs?
    // unsigned short m_capacity;      // how big is m_glyphs
    mutable GlyphFaceCache* m_pGlyphFaceCache;      //owned - never NULL
    mutable CmapCache* m_cmapCache; // cmap cache if available
    unsigned short m_upem;          // design units per em
protected:
    unsigned short m_numSilf;       // number of silf subtables in the silf table
    Silf *m_silfs;                   // silf subtables.
private:
    SillMap m_Sill;
    FileFace* m_pFileFace;      //owned
    mutable NameTable* m_pNames;
    
private:        //defensive on m_pGlyphFaceCache, m_pFileFace and m_silfs
    GrFace(const GrFace&);
    GrFace& operator=(const GrFace&);
};


inline bool FeatureRef::applyValToFeature(uint16 val, Features* pDest) const 
{ 
    if (val>m_max || !m_pFace)
      return false;
    if (pDest->m_pMap==NULL)
      pDest->m_pMap = &m_pFace->theSill().theFeatureMap();
    else
      if (pDest->m_pMap!=&m_pFace->theSill().theFeatureMap())
        return false;       //incompatible
    pDest->grow(m_index);
    {
        pDest->m_vec[m_index] &= ~m_mask;
        pDest->m_vec[m_index] |= (uint32(val) << m_bits);
    }
    return true;
}

inline uint16 FeatureRef::getFeatureVal(const Features& feats) const
{ 
  if (m_index < feats.m_length && &m_pFace->theSill().theFeatureMap()==feats.m_pMap) 
    return (feats.m_vec[m_index] & m_mask) >> m_bits; 
  else
    return 0;
}

}}}} // namespace
