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

#include "graphiteng/Types.h"

#ifdef __cplusplus
namespace org { namespace sil { namespace graphite { namespace v2 {
#endif

typedef struct GrFace GrFace;
typedef struct GrFont GrFont;
typedef struct GrFeatureRef GrFeatureRef;
typedef struct GrFeatureVal GrFeatureVal;

/**
 * The Face Options allow the application to require that certain tables are
 * read during face construction. This may be of concern if the appFaceHandle
 * used in the gr_get_table_fn may change.
 * The values can be combined 
 */
enum gr_faceOptions {
    /** No preload, no cmap caching, fail if the graphite tables are invalid */
    gr_face_default = 0,
    /** Dumb rendering will be enabled if the graphite tables are invalid */
    gr_face_dumb_rendering = 1,
    /** preload glyphs at construction time */
    gr_face_preloadGlyphs = 2,
    /** Cache the lookup from code point to glyph ID at construction time */
    gr_face_cacheCmap = 4
};

#ifdef __cplusplus
extern "C"
{
#endif
    typedef const void *(*gr_get_table_fn)(const void* appFaceHandle, unsigned int name, size_t *len);
    GRNG_EXPORT GrFace* gr_make_face(const void* appFaceHandle/*non-NULL*/, gr_get_table_fn getTable, unsigned int faceOptions);
                      //the appFaceHandle must stay alive all the time when the GrFace is alive. When finished with the GrFace, call face_destroy    
    GRNG_EXPORT GrFace* gr_make_face_with_seg_cache(const void* appFaceHandle/*non-NULL*/, gr_get_table_fn getTable, unsigned int segCacheMaxSize, unsigned int faceOptions);
                      //the appFaceHandle must stay alive all the time when the GrFace is alive. When finished with the GrFace, call face_destroy
    GRNG_EXPORT gr_uint32 gr_str_to_tag(const char *str);
    GRNG_EXPORT void gr_tag_to_str(gr_uint32 tag, char *str);

    GRNG_EXPORT GrFeatureVal* gr_face_featureval_for_lang(const GrFace* pFace, gr_uint32 langname/*0 means clone default*/); //clones the features. if none for language, clones the default. Call destroy_Features when done.
    GRNG_EXPORT const GrFeatureRef* gr_face_find_fref(const GrFace* pFace, gr_uint32 featId); //When finished with the FeatureRef, call features_destroy
    GRNG_EXPORT gr_uint16 gr_face_n_fref(const GrFace* pFace);
    GRNG_EXPORT const GrFeatureRef* gr_face_fref(const GrFace* pFace, gr_uint16 i); //When finished with the FeatureRef, call features_destroy
    GRNG_EXPORT unsigned short gr_face_n_languages(const GrFace* pFace);
    GRNG_EXPORT gr_uint32 gr_face_lang_by_index(const GrFace* pFace, gr_uint16 i);
    GRNG_EXPORT void gr_face_destroy(GrFace *face);

    GRNG_EXPORT unsigned short gr_face_n_glyphs(const GrFace* pFace);

#ifndef DISABLE_FILE_FACE
    GRNG_EXPORT GrFace* gr_make_file_face(const char *filename);   //returns NULL on failure. //TBD better error handling
                      //when finished with, call destroy_face
    GRNG_EXPORT GrFace* gr_make_file_face_with_seg_cache(const char *filename, unsigned int segCacheMaxSize, unsigned int faceOptions);   //returns NULL on failure. //TBD better error handling
#endif      //!DISABLE_FILE_FACE

    GRNG_EXPORT GrFont* gr_make_font(float ppm/*pixels per em*/, const GrFace *face/*needed for scaling, and the advance hints - must stay alive*/);
                      //When finished with the GrFont, call destroy_font    
    typedef float (*gr_advance_fn)(const void* appFontHandle, gr_uint16 glyphid);     //amount to advance. positive is in the writing direction
    GRNG_EXPORT GrFont* gr_make_font_with_advance_fn(float ppm/*pixels per em*/, const void* appFontHandle/*non-NULL*/, gr_advance_fn advance, const GrFace *face/*needed for scaling*/);
                      //the appFontHandle must stay alive all the time when the GrFont is alive. When finished with the GrFont, call destroy_font    
    GRNG_EXPORT void gr_font_destroy(GrFont *font);

    GRNG_EXPORT gr_uint16 gr_fref_feature_value(const GrFeatureRef*pfeatureref, const GrFeatureVal* feats);    //returns 0 if either pointer is NULL
    GRNG_EXPORT int gr_fref_set_feature_value(const GrFeatureRef* pfeatureref, gr_uint16 val, GrFeatureVal* pDest);    //returns false iff either pointer is NULL. or if they are not for the same face, or val is too big
    GRNG_EXPORT gr_uint32 gr_fref_id(const GrFeatureRef* pfeatureref);    //returns 0 if pointer is NULL

    
    //these two methods can be used to iterate over the possible values for a particular id. There may be gaps.
    GRNG_EXPORT gr_uint16 gr_fref_n_values(const GrFeatureRef* pfeatureref);
    GRNG_EXPORT gr_int16 gr_fref_value(const GrFeatureRef* pfeatureref, gr_uint16 settingno);   
    
    // Labels may not be available for requested langId, the language actually used
    // will be returned in langId. The length in bytes will be returned in length
    // call label_destroy when finished
    GRNG_EXPORT void* gr_fref_label(const GrFeatureRef* pfeatureref, gr_uint16 *langId, enum gr_encform utf, gr_uint32 *length);
    GRNG_EXPORT void* gr_fref_value_label(const GrFeatureRef* pfeatureref, gr_uint16 settingno/*rather than a value*/, gr_uint16 *langId, enum gr_encform utf, gr_uint32 *length);
    GRNG_EXPORT void gr_label_destroy(void * label);

    GRNG_EXPORT GrFeatureVal* gr_featureval_clone(GrFeatureVal* pfeatures/*may be NULL*/);
                      //When finished with the Features, call features_destroy    

    GRNG_EXPORT void gr_featureval_destroy(GrFeatureVal *pfeatures);

#ifdef __cplusplus
}
}}}} // namespace
#endif

