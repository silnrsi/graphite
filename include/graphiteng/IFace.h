#pragma once
#include <cassert>
#include "graphiteng/Types.h"
#include "graphiteng/FeaturesHandle.h"
#include "graphiteng/FeatureRefHandle.h"

/*
    A client would usually derive from IFace to implement their own way of hetting the table information for a font face.
    But if they are happy to load directly from a true type font from a file, they can use IFace::loadTTFFile instead, remebering to 
    delete the pointer when it is no longer required.

    Then the client should call the member function makeGrFace to load the IFace, and the GrFace pointer will then
    be passed into Graphite for further processing.
    GrFace is lazy and so the IFace must stay alive in case Graphite needs to get more data loaded into it.
    When the GrFace* is no longer needed, IFace::destroyGrFace() should be called.
*/
namespace org { namespace sil { namespace graphite { namespace v2 {

class GrFace;

class GRNG_EXPORT IFace
{
public:
    //virtual ~IFace() {}
    virtual const void *getTable(unsigned int name, size_t *len) const = 0;		//In standard TTF format. Must check in range. Return NULL if not.

    GrFace* makeGrFace() const;		//the 'this' must stay alive all the time when the GrFace is alive. When finished with the GrFace, call IFace::destroyGrFace    
    static FeaturesHandle getFeatures(const GrFace* pFace, uint32 langname/*0 means clone default*/); //clones the features. if none for language, clones the default
    static FeatureRefHandle feature(const GrFace* pFace, uint8 index);
    static void destroyGrFace(GrFace *face);

private :
#ifdef FIND_BROKEN_VIRTUALS
    virtual double getTable(unsigned int name, size_t *len) { return 0.0; }
#endif		//FIND_BROKEN_VIRTUALS
};

#ifndef DISABLE_FILE_FONT
class GRNG_EXPORT TtfFileFace : public IFace
{
public:
    void operator delete(void *);
    static TtfFileFace* loadTTFFile(const char *name);        //when no longer needed, call delete
                                //TBD better error handling
};
#endif      //!DISABLE_FILE_FONT

}}}} // namespace
