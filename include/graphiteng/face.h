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

enum EGlyphCacheStrategy{ eOneCache=0, eLoadOnDemand=100/*never unloaded*/, ePreload=200 } ;        //don't change numbering

class GRNG_EXPORT IFace
{
public:
    //virtual ~IFace() {}
    virtual const void *getTable(unsigned int name, size_t *len) const = 0;		//In standard TTF format. Must check in range. Return NULL if not.

    GrFace* makeGrFace(EGlyphCacheStrategy requestedStrategy=ePreload) const;		//the 'this' must stay alive all the time when the GrFace is alive. When finished with the GrFace, call IFace::destroyGrFace    
};

extern "C"
{
    GRNG_EXPORT FeaturesHandle get_features(const GrFace* pFace, uint32 langname/*0 means clone default*/); //clones the features. if none for language, clones the default
    GRNG_EXPORT FeatureRefHandle feature(const GrFace* pFace, uint8 index);
    GRNG_EXPORT void destroy_GrFace(GrFace *face);

    GRNG_EXPORT EGlyphCacheStrategy nearest_supported_strategy(EGlyphCacheStrategy requested);      //old implementations of graphite might not support a requested strategy 
    GRNG_EXPORT bool set_glyph_cache_strategy(const GrFace* pFace, EGlyphCacheStrategy requestedStrategy);       //glyphs already loaded are unloaded. return value indicates success. failure keeps old cache.
    GRNG_EXPORT EGlyphCacheStrategy get_glyph_strategy(const GrFace* pFace);
    GRNG_EXPORT unsigned short num_glyphs(const GrFace* pFace);
    GRNG_EXPORT unsigned long num_glyph_accesses(const GrFace* pFace);
    GRNG_EXPORT unsigned long num_glyph_loads(const GrFace* pFace);
}

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
