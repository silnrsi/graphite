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
typedef struct FeatureRef FeatureRef;
typedef struct Features Features;

#ifdef __cplusplus
extern "C"
{
#endif
    typedef const void *(*get_table_fn)(const void* appFaceHandle, unsigned int name, size_t *len);
    GRNG_EXPORT GrFace* make_face(const void* appFaceHandle/*non-NULL*/, get_table_fn getTable, int canDumb);
                      //the appFaceHandle must stay alive all the time when the GrFace is alive. When finished with the GrFace, call face_destroy    
    GRNG_EXPORT GrFace* make_face_with_seg_cache(const void* appFaceHandle/*non-NULL*/, get_table_fn getTable, unsigned int segCacheMaxSize, int canDumb);
                      //the appFaceHandle must stay alive all the time when the GrFace is alive. When finished with the GrFace, call face_destroy

    GRNG_EXPORT Features* face_features_for_lang(const GrFace* pFace, uint32 langname/*0 means clone default*/); //clones the features. if none for language, clones the default. Call destroy_Features when done.
    GRNG_EXPORT const FeatureRef* face_find_fref(const GrFace* pFace, uint32 featId); //When finished with the FeatureRef, call features_destroy
    GRNG_EXPORT uint16 face_n_fref(const GrFace* pFace);
    GRNG_EXPORT const FeatureRef* face_fref(const GrFace* pFace, uint16 i); //When finished with the FeatureRef, call features_destroy
    GRNG_EXPORT unsigned short face_n_languages(const GrFace* pFace);
    GRNG_EXPORT uint32 face_lang_by_index(const GrFace* pFace, uint16 i);
    GRNG_EXPORT void face_destroy(GrFace *face);

    GRNG_EXPORT unsigned short face_n_glyphs(const GrFace* pFace);

#ifndef DISABLE_FILE_FACE
    GRNG_EXPORT GrFace* make_file_face(const char *filename);   //returns NULL on failure. //TBD better error handling
                      //when finished with, call destroy_face
    GRNG_EXPORT GrFace* make_file_face_with_seg_cache(const char *filename, unsigned int segCacheMaxSize);   //returns NULL on failure. //TBD better error handling
#endif      //!DISABLE_FILE_FACE

    GRNG_EXPORT GrFont* make_font(float ppm/*pixels per em*/, const GrFace *face/*needed for scaling, and the advance hints - must stay alive*/);
                      //When finished with the GrFont, call destroy_font    
    typedef float (*advance_fn)(const void* appFontHandle, uint16 glyphid);     //amount to advance. positive is in the writing direction
    GRNG_EXPORT GrFont* make_font_with_advance_fn(float ppm/*pixels per em*/, const void* appFontHandle/*non-NULL*/, advance_fn advance, const GrFace *face/*needed for scaling*/);
                      //the appFontHandle must stay alive all the time when the GrFont is alive. When finished with the GrFont, call destroy_font    
    GRNG_EXPORT void font_destroy(GrFont *font);

    GRNG_EXPORT uint16 fref_feature_value(const FeatureRef*pfeatureref, const Features* feats);    //returns 0 if either pointer is NULL
    GRNG_EXPORT int fref_set_feature_value(const FeatureRef* pfeatureref, uint16 val, Features* pDest);    //returns false iff either pointer is NULL. or if they are not for the same face, or val is too big
    GRNG_EXPORT uint32 fref_id(const FeatureRef* pfeatureref);    //returns 0 if pointer is NULL

    
    //these two methods can be used to iterate over the possible values for a particular id. There may be gaps.
    GRNG_EXPORT uint16 fref_n_values(const FeatureRef*pfeatureref);
    GRNG_EXPORT int16 fref_value(const FeatureRef*pfeatureref, uint16 settingno);   
    
    // Labels may not be available for requested langId, the language actually used
    // will be returned in langId. The length in bytes will be returned in length
    // call label_destroy when finished
    GRNG_EXPORT void* fref_label(const FeatureRef*pfeatureref, uint16 *langId, enum encform utf, uint32 *length);
    GRNG_EXPORT void* fref_value_label(const FeatureRef*pfeatureref, uint16 settingno/*rather than a value*/, uint16 *langId, enum encform utf, uint32 *length);
    GRNG_EXPORT void label_destroy(void * label);

    GRNG_EXPORT Features* features_clone(Features* pfeatures/*may be NULL*/);
                      //When finished with the Features, call features_destroy    

    GRNG_EXPORT void features_destroy(Features *pfeatures);

#ifdef __cplusplus
}
}}}} // namespace
#endif

