#include <cstdio>
#include <map>
#include "graphiteng/IFace.h"

class FileFont : public IFace
{
public:
    FileFont(const char *name);
    virtual void *getTable(unsigned int name, size_t *len);

protected:
    FILE *m_pfile;
    std::map<unsigned int, std::pair<void *, size_t> > m_tables;
    char *m_pHeader;
    char *m_pTableDir;
};

