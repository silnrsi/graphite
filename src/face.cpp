#include "graphiteng/face.h"
#include "XmlTraceLog.h"
#include "GrFace.h"

#ifndef DISABLE_FILE_FONT
#include "TtfUtil.h"
#include <cstdio>
#include <cassert>
//#include <map> // Please don't use map, it forces libstdc++

using namespace org::sil::graphite::v2;

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


class org::sil::graphite::v2::TTFFaceHandle
{
public:
    TTFFaceHandle(const char *name);
    ~TTFFaceHandle();
//    virtual const void *getTable(unsigned int name, size_t *len) const;

    CLASS_NEW_DELETE
public:     //for local convenience
    FILE* m_pfile;
    mutable TableCacheItem m_tables[TtfUtil::ktiLast];
    char *m_pHeader;
    char *m_pTableDir;
    
private:		//defensive
    TTFFaceHandle(const TTFFaceHandle&);
    TTFFaceHandle& operator=(const TTFFaceHandle&);
};


TTFFaceHandle::TTFFaceHandle(const char *fname) :
    m_pHeader(NULL),
    m_pTableDir(NULL)
{
    if (!(m_pfile = fopen(fname, "rb"))) return;
    size_t lOffset, lSize;
    if (!TtfUtil::GetHeaderInfo(lOffset, lSize)) return;
    m_pHeader = gralloc<char>(lSize);
    if (fseek(m_pfile, lOffset, SEEK_SET)) return;
    if (fread(m_pHeader, 1, lSize, m_pfile) != lSize) return;
    if (!TtfUtil::CheckHeader(m_pHeader)) return;
    if (!TtfUtil::GetTableDirInfo(m_pHeader, lOffset, lSize)) return;
    m_pTableDir = gralloc<char>(lSize);
    if (fseek(m_pfile, lOffset, SEEK_SET)) return;
    if (fread(m_pTableDir, 1, lSize, m_pfile) != lSize) return;
}

TTFFaceHandle::~TTFFaceHandle()
{
    free(m_pTableDir);
    free(m_pHeader);
    if (m_pfile)
        fclose(m_pfile);
    m_pTableDir = NULL;
    m_pfile = NULL;
    m_pHeader = NULL;
}


static const void *TTFFaceHandle_table_fn(const void* appFaceHandle, unsigned int name, size_t *len)
{
    const TTFFaceHandle* ttfFaceHandle = (const TTFFaceHandle*)appFaceHandle;
    TableCacheItem * res;
    switch (name)
    {
        case tagCmap:
            res = &ttfFaceHandle->m_tables[TtfUtil::ktiCmap];
            break;
//        case tagCvt:
//            res = &m_tables[TtfUtil::ktiCvt];
//            break;
//        case tagCryp:FileFace
//            res = &m_tables[TtfUtil::ktiCryp];
//            break;
        case tagHead:
            res = &ttfFaceHandle->m_tables[TtfUtil::ktiHead];
            break;
//        case tagFpgm:
//            res = &m_tables[TtfUtil::ktiFpgm];
//            break;
//        case tagGdir:
//            res = &m_tables[TtfUtil::ktiGdir];
//            break;
        case tagGlyf:
            res = &ttfFaceHandle->m_tables[TtfUtil::ktiGlyf];
            break;
        case tagHdmx:
            res = &ttfFaceHandle->m_tables[TtfUtil::ktiHdmx];
            break;
        case tagHhea:
            res = &ttfFaceHandle->m_tables[TtfUtil::ktiHhea];
            break;
        case tagHmtx:
            res = &ttfFaceHandle->m_tables[TtfUtil::ktiHmtx];
            break;
        case tagLoca:
            res = &ttfFaceHandle->m_tables[TtfUtil::ktiLoca];
            break;
        case tagKern:
            res = &ttfFaceHandle->m_tables[TtfUtil::ktiKern];
            break;
//        case tagLtsh:
//            res = &m_tables[TtfUtil::ktiLtsh];
//            break;
        case tagMaxp:
            res = &ttfFaceHandle->m_tables[TtfUtil::ktiMaxp];
            break;
        case tagName:
            res = &ttfFaceHandle->m_tables[TtfUtil::ktiName];
            break;
        case tagOs2:
            res = &ttfFaceHandle->m_tables[TtfUtil::ktiOs2];
            break;
        case tagPost:
            res = &ttfFaceHandle->m_tables[TtfUtil::ktiPost];
            break;
//        case tagPrep:
//            res = &m_tables[TtfUtil::ktiPrep];
//            break;
        case tagFeat:
            res = &ttfFaceHandle->m_tables[TtfUtil::ktiFeat];
            break;
        case tagGlat:
            res = &ttfFaceHandle->m_tables[TtfUtil::ktiGlat];
            break;
        case tagGloc:
            res = &ttfFaceHandle->m_tables[TtfUtil::ktiGloc];
            break;
        case tagSilf:
            res = &ttfFaceHandle->m_tables[TtfUtil::ktiSilf];
            break;
        case tagSile:
            res = &ttfFaceHandle->m_tables[TtfUtil::ktiSile];
            break;
        case tagSill:
            res = &ttfFaceHandle->m_tables[TtfUtil::ktiSill];
            break;
        default:
            res = NULL;
    }
    assert(res); // don't expect any other table types
    if (!res) return NULL;
    if (res->data() == NULL)
    {
        char *tptr;
        size_t tlen, lOffset;
        if (!TtfUtil::GetTableInfo(name, ttfFaceHandle->m_pHeader, ttfFaceHandle->m_pTableDir, lOffset, tlen)) return NULL;
        if (fseek(ttfFaceHandle->m_pfile, lOffset, SEEK_SET)) return NULL;
        tptr = gralloc<char>(tlen);
        if (fread(tptr, 1, tlen, ttfFaceHandle->m_pfile) != tlen) return NULL;
        res->set(tptr, tlen);
    }
    if (len) *len = res->size();
    return res->data();
}
#endif			//!DISABLE_FILE_FONT

extern "C" 
{
    GRNG_EXPORT GrFace* make_GrFace(const void* appFaceHandle/*non-NULL*/, get_table_fn getTable, EGlyphCacheStrategy requestedStrategy)
                      //the appFaceHandle must stay alive all the time when the GrFace is alive. When finished with the GrFace, call destroy_GrFace    
    {
        GrFace *res = new GrFace(appFaceHandle, getTable);
    #ifndef DISABLE_TRACING
        XmlTraceLog::get().openElement(ElementFace);
    #endif
        bool valid = true;
        valid &= res->readGlyphs(requestedStrategy);
        valid &= res->readGraphite();
        valid &= res->readFeatures();
    #ifndef DISABLE_TRACING
        XmlTraceLog::get().closeElement(ElementFace);
    #endif
        
        if (!valid) {
            delete res;
            return 0;
        }
        return res;
    }
    

    GRNG_EXPORT FeaturesHandle get_features(const GrFace* pFace, uint32 langname/*0 means clone default*/) //clones the features. if none for language, clones the default
    {
        return pFace->theFeatures().cloneFeatures(langname);
    }


    GRNG_EXPORT FeatureRefHandle feature(const GrFace* pFace, uint8 index)
    {
        const FeatureRef* pRef = pFace->feature(index);
        if (!pRef)
            return NULL;

        return new FeatureRef(*pRef);
    }


    GRNG_EXPORT void destroy_GrFace(GrFace *face)
    {
        delete face;
    }


    GRNG_EXPORT EGlyphCacheStrategy nearest_supported_strategy(EGlyphCacheStrategy requested)      //old implementations of graphite might not support a requested strategy 
    {
        return GlyphFaceCache::nearestSupportedStrategy(requested);
    }


    GRNG_EXPORT bool set_glyph_cache_strategy(const GrFace* pFace, EGlyphCacheStrategy requestedStrategy)      //glyphs already loaded are unloaded
    {
        return pFace->setGlyphCacheStrategy(requestedStrategy);
    }


    GRNG_EXPORT EGlyphCacheStrategy get_glyph_strategy(const GrFace* pFace)
    {
        return pFace->getGlyphFaceCache()->getEnum();
    }


    GRNG_EXPORT unsigned short num_glyphs(const GrFace* pFace)
    {
        return pFace->getGlyphFaceCache()->numGlyphs();
    }


    GRNG_EXPORT unsigned long num_glyph_accesses(const GrFace* pFace)
    {
        return pFace->getGlyphFaceCache()->numAccesses();
    }


    GRNG_EXPORT unsigned long num_glyph_loads(const GrFace* pFace)
    {
        return pFace->getGlyphFaceCache()->numLoads();
    }


#ifndef DISABLE_FILE_FONT
    GRNG_EXPORT TTFFaceHandle* make_TTF_face_handle(const char *name)   //returns NULL on failure. //TBD better error handling
                      //when finished with, call destroy_TTF_face_handle
    {
        TTFFaceHandle* res = new TTFFaceHandle(name);
        if (res->m_pTableDir)
            return res;
        
        //error when loading

        delete res;
        return NULL;
    }
    
    
    GRNG_EXPORT void destroy_TTF_face_handle(TTFFaceHandle* appFaceHandle/*non-NULL*/)
    {
        delete appFaceHandle;
    }
    
    
    GRNG_EXPORT GrFace* make_GrFace_from_TTF_face_handle(const TTFFaceHandle* appFaceHandle/*non-NULL*/, EGlyphCacheStrategy requestedStrategy)
    {                  //the appFaceHandle must stay alive all the time when the GrFace is alive. When finished with the GrFace, call destroy_TTF_face_handle 
        return make_GrFace(appFaceHandle/*non-NULL*/, &TTFFaceHandle_table_fn, requestedStrategy);
    }
#endif      //!DISABLE_FILE_FONT
}



