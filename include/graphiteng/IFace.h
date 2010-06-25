#ifndef IFACE_INCLUDE
#define IFACE_INCLUDE

#include "graphiteng/Types.h"

/*
    A client would usually derive from IFace to implement their own way of hetting the table information for a font face.
    But if they are happy to load directly from a true type font from a file, they can use IFace::loadTTFFile instead, remebering to 
    delete the pointer when it is no longer required.

    Then the client should call the member function makeLoadedFace to load the IFace, and the LoadedFace pointer will then
    be passed into Graphite for further processing.
    LoadedFace is lazy and so the IFace must stay alive in case Graphite needs to get more data loaded into it.
    When the LoadedFace* is no longer needed, IFace::destroyLoadedFace() should be called.
*/

class LoadedFace;


class GRNG_EXPORT IFace
{
public:
    virtual ~IFace() {}
    virtual const void *getTable(unsigned int name, size_t *len) const = 0;		//In standard TTF format
    
#ifndef DISABLE_FILE_FONT
    static IFace* loadTTFFile(const char *name);		//when no longer needed, call delete
								//TBD better error handling
#endif 		//!DISABLE_FILE_FONT


    LoadedFace* makeLoadedFace() const;		//this must stay alive all the time when the LoadedFace is alive. When finished with the LoadeFace, call IFace::destroyLoadedFace
    static void destroyLoadedFace(LoadedFace *face);

private :
#ifdef FIND_BROKEN_VIRTUALS
    virtual double getTable(unsigned int name, size_t *len) { return 0.0; }
#endif		//FIND_BROKEN_VIRTUALS
};

#endif

