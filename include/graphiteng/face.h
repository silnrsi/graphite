#pragma once
#include <cassert>
#include "graphiteng/Types.h"
#include "graphiteng/FeaturesHandle.h"
#include "graphiteng/FeatureRefHandle.h"

namespace org { namespace sil { namespace graphite { namespace v2 {

class GrFace;
class TTFFaceHandle;

enum EGlyphCacheStrategy{ eOneCache=0, eLoadOnDemand=100/*never unloaded*/, ePreload=200 } ;        //don't change numbering

extern "C"
{
    typedef const void *(*get_table_fn)(const void* appFaceHandle, unsigned int name, size_t *len);
    GRNG_EXPORT GrFace* make_GrFace(const void* appFaceHandle/*non-NULL*/, get_table_fn getTable, EGlyphCacheStrategy requestedStrategy);
                      //the appFaceHandle must stay alive all the time when the GrFace is alive. When finished with the GrFace, call destroy_GrFace    
  
    GRNG_EXPORT FeaturesHandle get_features(const GrFace* pFace, uint32 langname/*0 means clone default*/); //clones the features. if none for language, clones the default
    GRNG_EXPORT FeatureRefHandle feature(const GrFace* pFace, uint8 index);
    GRNG_EXPORT void destroy_GrFace(GrFace *face);

    GRNG_EXPORT EGlyphCacheStrategy nearest_supported_strategy(EGlyphCacheStrategy requested);      //old implementations of graphite might not support a requested strategy 
    GRNG_EXPORT bool set_glyph_cache_strategy(const GrFace* pFace, EGlyphCacheStrategy requestedStrategy);       //glyphs already loaded are unloaded. return value indicates success. failure keeps old cache.
    GRNG_EXPORT EGlyphCacheStrategy get_glyph_strategy(const GrFace* pFace);
    GRNG_EXPORT unsigned short num_glyphs(const GrFace* pFace);
    GRNG_EXPORT unsigned long num_glyph_accesses(const GrFace* pFace);
    GRNG_EXPORT unsigned long num_glyph_loads(const GrFace* pFace);

#ifndef DISABLE_FILE_FONT
    GRNG_EXPORT TTFFaceHandle* make_TTF_face_handle(const char *name);   //returns NULL on failure. //TBD better error handling
                      //when finished with, call destroy_TTF_face_handle
    GRNG_EXPORT void destroy_TTF_face_handle(TTFFaceHandle* appFaceHandle/*non-NULL*/);
    GRNG_EXPORT GrFace* make_GrFace_from_TTF_face_handle(const TTFFaceHandle* appFaceHandle/*non-NULL*/, EGlyphCacheStrategy requestedStrategy);
                      //the appFaceHandle must stay alive all the time when the GrFace is alive. When finished with the GrFace, call destroy_TTF_face_handle    
#endif      //!DISABLE_FILE_FONT
}

}}}} // namespace
