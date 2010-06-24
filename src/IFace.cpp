#include "graphiteng/IFace.h"
#include "XmlTraceLog.h"
#include "LoadedFace.h"

#ifndef DISABLE_FILE_FONT
#include "TtfUtil.h"
#include <cstdio>
#include <map>

class FileFont /*really a FileFace!*/: public IFace
{
friend class IFace;

public:
    FileFont(const char *name);
    ~FileFont();
    virtual const void *getTable(unsigned int name, size_t *len) const;

private:
    FILE* m_pfile;
    mutable std::map<unsigned int, std::pair<const void *, size_t> > m_tables;
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
    fclose(m_pfile);
}

const void *FileFont::getTable(unsigned int name, size_t *len) const
{
    std::map<unsigned int, std::pair<const void *, size_t> >::const_iterator res;
    if ((res = m_tables.find(name)) == m_tables.end())
    {
        void *tptr;
        size_t tlen, lOffset;
        if (!TtfUtil::GetTableInfo(name, m_pHeader, m_pTableDir, lOffset, tlen)) return NULL;
        if (fseek(m_pfile, lOffset, SEEK_SET)) return NULL;
        tptr = new char[tlen];
        if (fread(tptr, 1, tlen, m_pfile) != tlen) return NULL;
//        if (!TtfUtil::CheckTable(name, tptr, tlen)) return NULL;

        std::pair<unsigned int, std::pair<const void *, size_t> > kvalue = std::pair<unsigned int, std::pair<const void *, size_t> >(name, std::pair<void *, size_t>(tptr, tlen));
        std::pair<std::map<unsigned int, std::pair<const void *, size_t> >::iterator, bool> result = m_tables.insert(kvalue);
        if (result.second)
            res = result.first;
    }
    if (len) *len = res->second.second;
    return res->second.first;
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


/*static*/ void IFace::destroyLoadedFace(LoadedFace *face)
{
    delete face;
}




