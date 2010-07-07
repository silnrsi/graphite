#include "graphiteng/IFace.h"
#include "XmlTraceLog.h"
#include "LoadedFace.h"

#ifndef DISABLE_FILE_FONT
#include "TtfUtil.h"
#include <cstdio>
#include <cassert>
//#include <map> // Please don't use map, it forces libstdc++

class TableCacheItem
{
public:
    TableCacheItem(char * theData, size_t theSize) : m_data(theData), m_size(theSize) {}
    TableCacheItem() : m_data(0), m_size(0) {}
    ~TableCacheItem()
    {
        if (m_size) delete m_data;
    }
    void set(char * theData, size_t theSize) { m_data = theData; m_size = theSize; }
    const void * data() const { return m_data; }
    size_t size() const { return m_size; }
private:
    char * m_data;
    size_t m_size;
};


class FileFont /*really a FileFace!*/: public IFace
{
friend class IFace;

public:
    FileFont(const char *name);
    ~FileFont();
    virtual const void *getTable(unsigned int name, size_t *len) const;

private:
    FILE* m_pfile;
//    mutable std::map<unsigned int, std::pair<const void *, size_t> > m_tables;
    mutable TableCacheItem m_tables[TtfUtil::ktiLast];
    char *m_pHeader;
    char *m_pTableDir;
    
private:		//defensive
    FileFont(const FileFont&);
    FileFont& operator=(const FileFont&);
};


FileFont::FileFont(const char *fname) :
    m_pHeader(NULL),
    m_pTableDir(NULL)
{
    if (!(m_pfile = fopen(fname, "rb"))) return;
    size_t lOffset, lSize;
    if (!TtfUtil::GetHeaderInfo(lOffset, lSize)) return;
    m_pHeader = new char[lSize];
    if (fseek(m_pfile, lOffset, SEEK_SET)) return;
    if (fread(m_pHeader, 1, lSize, m_pfile) != lSize) return;
    if (!TtfUtil::CheckHeader(m_pHeader)) return;
    if (!TtfUtil::GetTableDirInfo(m_pHeader, lOffset, lSize)) return;
    m_pTableDir = new char[lSize];
    if (fseek(m_pfile, lOffset, SEEK_SET)) return;
    if (fread(m_pTableDir, 1, lSize, m_pfile) != lSize) return;
}

FileFont::~FileFont()
{
    delete[] m_pTableDir;
    delete[] m_pHeader;
    if (m_pfile)
        fclose(m_pfile);
    m_pTableDir = NULL;
    m_pfile = NULL;
    m_pHeader = NULL;
}

const void *FileFont::getTable(unsigned int name, size_t *len) const
{
//    std::map<unsigned int, std::pair<const void *, size_t> >::const_iterator res;
//    if ((res = m_tables.find(name)) == m_tables.end())
    TableCacheItem * res;
    switch (name)
    {
        case tagCmap:
            res = &m_tables[TtfUtil::ktiCmap];
            break;
//        case tagCvt:
//            res = &m_tables[TtfUtil::ktiCvt];
//            break;
//        case tagCryp:
//            res = &m_tables[TtfUtil::ktiCryp];
//            break;
        case tagHead:
            res = &m_tables[TtfUtil::ktiHead];
            break;
//        case tagFpgm:
//            res = &m_tables[TtfUtil::ktiFpgm];
//            break;
//        case tagGdir:
//            res = &m_tables[TtfUtil::ktiGdir];
//            break;
        case tagGlyf:
            res = &m_tables[TtfUtil::ktiGlyf];
            break;
        case tagHdmx:
            res = &m_tables[TtfUtil::ktiHdmx];
            break;
        case tagHhea:
            res = &m_tables[TtfUtil::ktiHhea];
            break;
        case tagHmtx:
            res = &m_tables[TtfUtil::ktiHmtx];
            break;
        case tagLoca:
            res = &m_tables[TtfUtil::ktiLoca];
            break;
        case tagKern:
            res = &m_tables[TtfUtil::ktiKern];
            break;
//        case tagLtsh:
//            res = &m_tables[TtfUtil::ktiLtsh];
//            break;
        case tagMaxp:
            res = &m_tables[TtfUtil::ktiMaxp];
            break;
        case tagName:
            res = &m_tables[TtfUtil::ktiName];
            break;
        case tagOs2:
            res = &m_tables[TtfUtil::ktiOs2];
            break;
        case tagPost:
            res = &m_tables[TtfUtil::ktiPost];
            break;
//        case tagPrep:
//            res = &m_tables[TtfUtil::ktiPrep];
//            break;
        case tagFeat:
            res = &m_tables[TtfUtil::ktiFeat];
            break;
        case tagGlat:
            res = &m_tables[TtfUtil::ktiGlat];
            break;
        case tagGloc:
            res = &m_tables[TtfUtil::ktiGloc];
            break;
        case tagSilf:
            res = &m_tables[TtfUtil::ktiSilf];
            break;
        case tagSile:
            res = &m_tables[TtfUtil::ktiSile];
            break;
        case tagSill:
            res = &m_tables[TtfUtil::ktiSill];
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
        if (!TtfUtil::GetTableInfo(name, m_pHeader, m_pTableDir, lOffset, tlen)) return NULL;
        if (fseek(m_pfile, lOffset, SEEK_SET)) return NULL;
        tptr = new char[tlen];
        if (fread(tptr, 1, tlen, m_pfile) != tlen) return NULL;
//        if (!TtfUtil::CheckTable(name, tptr, tlen)) return NULL;
        res->set(tptr, tlen);
        
        //std::pair<unsigned int, std::pair<const void *, size_t> > kvalue = std::pair<unsigned int, std::pair<const void *, size_t> >(name, std::pair<void *, size_t>(tptr, tlen));
        //std::pair<std::map<unsigned int, std::pair<const void *, size_t> >::iterator, bool> result = m_tables.insert(kvalue);
        //if (result.second)
        //    res = result.first;
    }
    //if (len) *len = res->second.second;
    //return res->second.first;
    if (len) *len = res->size();
    return res->data();
}

/*static*/ IFace* IFace::loadTTFFile(const char *name)		//when no longer needed, call delete
{
    FileFont* res = new FileFont(name);
    if (res->m_pTableDir)
	return res;
    
    //error when loading
    delete res;
    return NULL;
}
#endif			//!DISABLE_FILE_FONT

void IFace::operator delete(void* p, size_t)
{
    ::delete(p);
}


LoadedFace* IFace::makeLoadedFace() const		//this must stay alive all the time when the LoadedFace is alive. When finished with the LoadeFace, call IFace::destroyLoadedFace
{
    LoadedFace *res = new LoadedFace(this);
#ifndef DISABLE_TRACING
    XmlTraceLog::get().openElement(ElementFace);
#endif
    bool valid = true;
    valid &= res->readGlyphs();
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

/*static*/ FeaturesHandle IFace::getFeatures(const LoadedFace* pFace, uint32 langname/*0 means clone default*/) //clones the features. if none for language, clones the default
{
    return pFace->theFeatures().cloneFeatures(langname);
}


/*static*/ FeatureRefHandle IFace::feature(const LoadedFace* pFace, uint8 index)
{
    const FeatureRef* pRef = pFace->feature(index);
    if (!pRef)
	return NULL;
    
    return new FeatureRef(*pRef);
}


/*static*/ void IFace::destroyLoadedFace(LoadedFace *face)
{
    delete face;
}




