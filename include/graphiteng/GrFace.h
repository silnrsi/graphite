#pragma once
#include <cassert>
#include "graphiteng/Types.h"
#include "graphiteng/Features.h"
#include "graphiteng/FeatureRef.h"

namespace org { namespace sil { namespace graphite { namespace v2 {

class GrFace;

enum EGlyphCacheStrategy{ eOneCache=0, eLoadOnDemand=100/*never unloaded*/, ePreload=200 } ;        //don't change numbering

extern "C"
{
    typedef const void *(*get_table_fn)(const void* appFaceHandle, unsigned int name, size_t *len);
    GRNG_EXPORT GrFace* make_face(const void* appFaceHandle/*non-NULL*/, get_table_fn getTable, EGlyphCacheStrategy requestedStrategy, bool canDumb = false);
                      //the appFaceHandle must stay alive all the time when the GrFace is alive. When finished with the GrFace, call destroy_face    
  
    GRNG_EXPORT Features* face_features_for_lang(const GrFace* pFace, uint32 langname/*0 means clone default*/); //clones the features. if none for language, clones the default. Call features_destroy when done.
    GRNG_EXPORT uint8 face_n_fref(const GrFace* pFace);
    GRNG_EXPORT const FeatureRef* face_fref(const GrFace* pFace, uint8 index/*<face_n_fref*/);  //the FeatureRef is owned by the face
    GRNG_EXPORT const FeatureRef* face_find_fref(const GrFace* pFace, uint32 name);  //returns NULL if not found. The FeatureRef is owned by the face

    GRNG_EXPORT void destroy_face(GrFace *face);

    GRNG_EXPORT EGlyphCacheStrategy nearest_supported_strategy(EGlyphCacheStrategy requested);      //old implementations of graphite might not support a requested strategy 
    GRNG_EXPORT bool set_face_glyph_cache_strategy(const GrFace* pFace, EGlyphCacheStrategy requestedStrategy);       //glyphs already loaded are unloaded. return value indicates success. failure keeps old cache.
    GRNG_EXPORT EGlyphCacheStrategy face_glyph_strategy(const GrFace* pFace);
    GRNG_EXPORT unsigned short face_n_glyphs(const GrFace* pFace);
    GRNG_EXPORT unsigned long face_n_glyph_accesses(const GrFace* pFace);
    GRNG_EXPORT unsigned long face_n_glyph_loads(const GrFace* pFace);

#ifndef DISABLE_FILE_FACE
    GRNG_EXPORT GrFace* make_file_face(const char *filename, EGlyphCacheStrategy requestedStrategy);   //returns NULL on failure. //TBD better error handling
                      //when finished with, call destroy_face
#endif      //!DISABLE_FILE_FACE
}

}}}} // namespace
