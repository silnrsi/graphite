#pragma once

#include "graphiteng/Types.h"
#include "graphiteng/GrSegment.h"

namespace org { namespace sil { namespace graphite { namespace v2 {

typedef uint32 FeatId, FeatVal, FeatVec;

class GrFace;
class GrSegment;
class GrFont;
  
extern "C"
{
    GRNG_EXPORT size_t face_get_n_feat_vec(const GrFace* pFace/*not NULL*/);

    GRNG_EXPORT size_t face_get_n_feat_id(const GrFace* pFace/*not NULL*/, size_t vec_index/*<face_get_n_feat_vec(pFace)*/);
    GRNG_EXPORT FeatId/*not zero - except if args are bad*/ face_get_feat_id(const GrFace* pFace/*not NULL*/, size_t vec_index/*<face_get_n_feat_vec(pFace)*/, size_t id_index/*<face_get_n_feat_id(pFace, vec_index)*/);
                        //FeatId's are specific to a particular GrFace and vec_index. If you & two FeatId's for the same
                        //GrFace and vec_index, but different id_index, you will get 0.
    
    GRNG_EXPORT FeatVec face_get_std_feat_vec_for_lang(const GrFace* pFace/*not NULL*/, uint32 langname/*0 means default*/, size_t vec_index/*<face_get_n_feat_vec(pFace)*/);
    GRNG_EXPORT FeatVal feat_vec_get_feat_val(FeatVec vec, FeatId id);
                        //vec and id should have come from the same GrFace and vec_index.
                        //If you | the result with id, you will always get id.
                        
    GRNG_EXPORT FeatVec feat_vec_set_feat_val(FeatVec vec, FeatId id, FeatVal val);
                        //vec and id should have come from the same GrFace and vec_index.
                        //result is the same as vec for feat_vec_get_feat_val for all other id's and with 
                        //feat_vec_get_feat_val(result, id)==(id&val)
                        
    GRNG_EXPORT GrSegment* seg_make_with_feats(const GrFont* font, const GrFace* pFace/*not NULL*/, uint32 script, size_t nVecs, const FeatVec* pFeatVecs/*if nVecs!=0, this is not NULL*/, encform enc, const void* pStart, size_t nChars, int dir);
                        //pFeatVecs points to a sequence of nVecs FeatVec's in the same order as from face_get_n_feat_id for pFace.
                        //extra FeatVecs are ignored, and omitted opnes at the end take the default value.


    //Now some stuff for being able to display the choices in menus.
     GRNG_EXPORT size_t face_get_n_langs_for_naming_feats(const GrFace* pFace/*not NULL*/);
                        //not every feaure needs to be able to name all the features
                        
     GRNG_EXPORT size_t/*number of unicode chars*/ face_get_feat_id_name(const GrFace* pFace/*not NULL*/, size_t vec_index/*<face_get_n_feat_vec(pFace)*/, FeatId id/*came from face_get_feat_id*/,
                                              uint32 langname/*0 means default, which can name everything with a name*/, encform enc, void* buffer_begin, void* buffer_end);
                        //result is 0, if the id does not have a name in langname (according to pFace).
              
 
     GRNG_EXPORT size_t face_get_n_feat_val(const GrFace* pFace/*not NULL*/, size_t vec_index/*<face_get_n_feat_vec(pFace)*/, FeatId id/*came from face_get_feat_id*/, bool* pCanUseCheckBox/*not NULL*/);
                        //most feat_id can have 2 values and would be displayed with a checkbox
                        
     GRNG_EXPORT size_t/*number of unicode chars*/ face_get_n_feat_val_names(const GrFace* pFace/*not NULL*/, size_t vec_index/*<face_get_n_feat_vec(pFace)*/, FeatId id/*came from face_get_feat_id*/, 
                                                                             size_t val_index/*<face_get_n_feat_val*/, FeatVal* pIndexedVal/*not NULL. set to the value being named*/,
                                                                             uint32 langname/*0 means default, which can name everything with a name*/, encform enc, void* buffer_begin, void* buffer_end);
}


}}}} // namespace
