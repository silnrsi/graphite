#ifndef IFACE_INCLUDE
#define IFACE_INCLUDE

#include "graphiteng/Types.h"

class GRNG_EXPORT IFace
{
public:
    virtual ~IFace() {}
    virtual const void *getTable(unsigned int name, size_t *len) const = 0;		//TBD document purpose/reqd format of return value
    
#ifndef DISABLE_FILE_FONT
    static IFace* loadTTFFile(const char *name);		//when no longer needed, call delete
								//TBD better error handling
#endif 		//!DISABLE_FILE_FONT
    
private :
#ifdef FIND_BROKEN_VIRTUALS
    virtual double getTable(unsigned int name, size_t *len) { return 0.0; }
#endif		//FIND_BROKEN_VIRTUALS
};

#endif

