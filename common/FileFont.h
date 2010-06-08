#include <cstdio>
#include <map>
#include "FontFace.h"

class FileFont : public FontFace
{
public:
    FileFont(char *name);
    void *getTable(TableId name, size_t &len);

protected:
    FILE *m_pfile;
    std::map<TableId, std::pair<void *, size_t> > m_tables;
    char *m_pHeader;
    char *m_pTableDir;
};

