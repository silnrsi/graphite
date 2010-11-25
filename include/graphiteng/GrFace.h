/*  GRAPHITENG LICENSING

    Copyright 2010, SIL International
    All rights reserved.

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published
    by the Free Software Foundation; either version 2.1 of License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should also have received a copy of the GNU Lesser General Public
    License along with this library in the file named "LICENSE".
    If not, write to the Free Software Foundation, Inc., 59 Temple Place,
    Suite 330, Boston, MA 02111-1307, USA or visit their web page on the
    internet at http://www.fsf.org/licenses/lgpl.html.
*/
#pragma once
#include <cassert>
#include "graphiteng/Types.h"
#include "graphiteng/Features.h"
#include "graphiteng/FeatureRef.h"

namespace org { namespace sil { namespace graphite { namespace v2 {

class GrFace;

enum EGlyphCacheStrategy{
    eOneCache=0x0,
    eLoadOnDemand=0x100/*never unloaded*/,
    ePreload=0x200,
    eLoadMask=0xF00, /* for internal use */
    eCmap=0x1000,
    eLoadOnDemandWithCmap=eCmap+eLoadOnDemand,
    ePreloadWithCmap=eCmap+ePreload
};  //don't change numbering

extern "C"
{
    typedef const void *(*get_table_fn)(const void* appFaceHandle, unsigned int name, size_t *len);
    GRNG_EXPORT GrFace* make_face(const void* appFaceHandle/*non-NULL*/, get_table_fn getTable, EGlyphCacheStrategy requestedStrategy, bool canDumb = false);
                      //the appFaceHandle must stay alive all the time when the GrFace is alive. When finished with the GrFace, call destroy_face    
    GRNG_EXPORT GrFace* make_face_with_seg_cache(const void* appFaceHandle/*non-NULL*/, get_table_fn getTable, EGlyphCacheStrategy requestedStrategy, unsigned int segCacheMaxSize, bool canDumb = false);
                      //the appFaceHandle must stay alive all the time when the GrFace is alive. When finished with the GrFace, call destroy_face

    GRNG_EXPORT Features* face_features_for_lang(const GrFace* pFace, uint32 langname/*0 means clone default*/); //clones the features. if none for language, clones the default. Call destroy_Features when done.
    GRNG_EXPORT FeatureRef* face_feature_ref(const GrFace* pFace, uint32 featId); //When finished with the FeatureRef, call features_destroy
    GRNG_EXPORT unsigned short face_num_features(const GrFace* pFace);
    GRNG_EXPORT FeatureRef* face_feature_by_index(const GrFace* pFace, uint16 i); //When finished with the FeatureRef, call features_destroy
    GRNG_EXPORT unsigned short face_num_languages(const GrFace* pFace);
    GRNG_EXPORT uint32 face_lang_by_index(const GrFace* pFace, uint16 i);
    GRNG_EXPORT void destroy_face(GrFace *face);

    GRNG_EXPORT EGlyphCacheStrategy nearest_supported_strategy(EGlyphCacheStrategy requested);      //old implementations of graphite might not support a requested strategy 
    GRNG_EXPORT bool set_face_glyph_cache_strategy(const GrFace* pFace, EGlyphCacheStrategy requestedStrategy);       //glyphs already loaded are unloaded. return value indicates success. failure keeps old cache.
    GRNG_EXPORT EGlyphCacheStrategy face_glyph_strategy(const GrFace* pFace);
    GRNG_EXPORT unsigned short face_num_glyphs(const GrFace* pFace);
    GRNG_EXPORT unsigned long face_num_glyph_accesses(const GrFace* pFace);
    GRNG_EXPORT unsigned long face_num_glyph_loads(const GrFace* pFace);

#ifndef DISABLE_FILE_FACE
    GRNG_EXPORT GrFace* make_file_face(const char *filename, EGlyphCacheStrategy requestedStrategy);   //returns NULL on failure. //TBD better error handling
                      //when finished with, call destroy_face
    GRNG_EXPORT GrFace* make_file_face_with_seg_cache(const char *filename, EGlyphCacheStrategy requestedStrategy, unsigned int segCacheMaxSize);   //returns NULL on failure. //TBD better error handling
#endif      //!DISABLE_FILE_FACE
}

}}}} // namespace
