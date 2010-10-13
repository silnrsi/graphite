#pragma once

#include "graphiteng/Types.h"
#include "graphiteng/SlotHandle.h"

namespace org { namespace sil { namespace graphite { namespace v2 {

class GrSegment;
class GrFace;
class FeaturesHandle;
class CharInfo;


enum encform {
  kutf8 = 1/*sizeof(uint8)*/, kutf16 = 2/*sizeof(uint16)*/, kutf32 = 4/*sizeof(uint32)*/
};


extern "C"
{
    //in the following methods, pError may be NULL. if it is not NULL, and the buffer contains some bad bytes not in the utf range, then *pError is set to point at the first bad byte. Otherwise, *pError is set to NULL.
    size_t count_unicode_characters(encform enc, const void* buffer_begin, const void* buffer_end/*as in stl i.e. don't use end*/, const void** pError);
    size_t count_unicode_characters_with_max_count(encform enc, const void* buffer_begin, const void* buffer_end/*as in stl i.e. don't use end*/, size_t maxCount, const void** pError);
    size_t count_unicode_characters_to_nul(encform enc, const void* buffer_begin, const void** pError);  //the nul is not in the count
    size_t count_unicode_characters_to_nul_or_end(encform enc, const void* buffer_begin, const void* buffer_end/*don't go on or past end*/, const void** pError);   //the nul is not in the count
    size_t count_unicode_characters_to_nul_or_end_with_max_count(encform enc, const void* buffer_begin, const void* buffer_end/*don't go on or past end*/, size_t maxCount, const void** pError);  //the nul is not in the count
}


extern "C"
{
    GRNG_EXPORT GrSegment* make_GrSegment(const GrFont* font, const GrFace* face, uint32 script, encform enc, const void* pStart, size_t nChars, int dir);
                      //When finished with the GrFont, call destroy_GrSegment    
    GRNG_EXPORT GrSegment* make_GrSegment_using_features(const GrFont* font, const GrFace* face, uint32 script, const FeaturesHandle& pFeats, encform enc, const void* pStart, size_t nChars, int dir);
                      //When finished with the GrFont, call destroy_GrSegment    
    GRNG_EXPORT void destroy_GrSegment(GrSegment* p);

    GRNG_EXPORT int length(const GrSegment* pSeg/*not NULL*/);
    GRNG_EXPORT float advance_X(const GrSegment* pSeg/*not NULL*/);
    GRNG_EXPORT float advance_Y(const GrSegment* pSeg/*not NULL*/);
    GRNG_EXPORT CharInfo* charInfo(GrSegment* pSeg/*not NULL*/, int index);
    GRNG_EXPORT void run_graphite(GrSegment* pSeg/*not NULL*/);
    GRNG_EXPORT void choose_silf(GrSegment* pSeg/*not NULL*/, uint32 script);
    GRNG_EXPORT SlotHandle first(GrSegment* pSeg/*not NULL*/);
    
    GRNG_EXPORT int add_features(GrSegment* pSeg/*not NULL*/, const FeaturesHandle& feats);
}
  
  
}}}} // namespace
