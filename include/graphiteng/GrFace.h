#pragma once
#include <cassert>
#include "graphiteng/Types.h"
#include "graphiteng/Features.h"
#include "graphiteng/FeatureRef.h"

namespace org { namespace sil { namespace graphite { namespace v2 {

class GrFace;
class FileFaceHandle;

enum EGlyphCacheStrategy{ eOneCache=0x0, eLoadOnDemand=0x100/*never unloaded*/, ePreload=0x200 } ;        //don't change numbering

extern "C"
{
    typedef const void *(*get_table_fn)(const void* appFaceHandle, unsigned int name, size_t *len);
    GRNG_EXPORT GrFace* make_GrFace(const void* appFaceHandle/*non-NULL*/, get_table_fn getTable, EGlyphCacheStrategy requestedStrategy, bool canDumb = false);
                      //the appFaceHandle must stay alive all the time when the GrFace is alive. When finished with the GrFace, call destroy_GrFace    

    GRNG_EXPORT Features* get_features(const GrFace* pFace, uint32 langname/*0 means clone default*/); //clones the features. if none for language, clones the default. Call destroy_Features when done.
    GRNG_EXPORT FeatureRef* feature(const GrFace* pFace, uint8 index);  //When finished with the FeatureRef, call destroy_FeatureRef
    GRNG_EXPORT void destroy_GrFace(GrFace *face);

    GRNG_EXPORT EGlyphCacheStrategy nearest_supported_strategy(EGlyphCacheStrategy requested);      //old implementations of graphite might not support a requested strategy 
    GRNG_EXPORT bool set_glyph_cache_strategy(const GrFace* pFace, EGlyphCacheStrategy requestedStrategy);       //glyphs already loaded are unloaded. return value indicates success. failure keeps old cache.
    GRNG_EXPORT EGlyphCacheStrategy get_glyph_strategy(const GrFace* pFace);
    GRNG_EXPORT unsigned short num_glyphs(const GrFace* pFace);
    GRNG_EXPORT unsigned long num_glyph_accesses(const GrFace* pFace);
    GRNG_EXPORT unsigned long num_glyph_loads(const GrFace* pFace);

    GRNG_EXPORT void enable_segment_cache(GrFace* pFace, size_t maxSegments, uint32 flags = 0); // flags is reserved for future use

#ifndef DISABLE_FILE_FACE_HANDLE
    GRNG_EXPORT FileFaceHandle* make_file_face_handle(const char *filename);   //returns NULL on failure. //TBD better error handling
                      //when finished with, call destroy_file_face_handle
    GRNG_EXPORT void destroy_file_face_handle(FileFaceHandle* appFaceHandle/*non-NULL*/);
    GRNG_EXPORT GrFace* make_GrFace_from_file_face_handle(const FileFaceHandle* appFaceHandle/*non-NULL*/, EGlyphCacheStrategy requestedStrategy);
                      //the appFaceHandle must stay alive all the time when the GrFace is alive. When finished with the GrFace, call destroy_TTF_face_handle    
#endif      //!DISABLE_FILE_FACE_HANDLE
}

}}}} // namespace
