#pragma once

#include "graphiteng/Types.h"

namespace org { namespace sil { namespace graphite { namespace v2 {

class GrSegment;
class GrFace;
class CharInfo;
class Features;
class Slot;
class GrFont;


enum encform {
  kutf8 = 1/*sizeof(uint8)*/, kutf16 = 2/*sizeof(uint16)*/, kutf32 = 4/*sizeof(uint32)*/
};


extern "C"
{
    //in the following method, pError may be NULL. if it is not NULL, and the buffer contains some bad bytes not in the utf range, then *pError is set to point at the first bad byte. Otherwise, *pError is set to NULL.
    size_t count_unicode_characters(encform enc, const void* buffer_begin, const void* buffer_end/*don't go on or past end, If NULL then ignored*/, const void** pError);   //Also stops on nul. Any nul is not in the count
}


extern "C"
{
    GRNG_EXPORT GrSegment* make_GrSegment(const GrFont* font, const GrFace* face, uint32 script, encform enc, const void* pStart, size_t nChars, int dir);
                      //When finished with the GrFont, call destroy_GrSegment    
    GRNG_EXPORT GrSegment* make_GrSegment_using_features(const GrFont* font, const GrFace* face, uint32 script, const Features* pFeats, encform enc, const void* pStart, size_t nChars, int dir);
                      //When finished with the GrFont, call destroy_GrSegment    
    GRNG_EXPORT void destroy_GrSegment(GrSegment* p);

    GRNG_EXPORT float advance_X(const GrSegment* pSeg/*not NULL*/);
    GRNG_EXPORT float advance_Y(const GrSegment* pSeg/*not NULL*/);
    GRNG_EXPORT unsigned int number_of_CharInfo(const GrSegment* pSeg/*not NULL*/);
    GRNG_EXPORT const CharInfo* charInfo(const GrSegment* pSeg/*not NULL*/, unsigned int index/*must be <number_of_CharInfo*/);
    GRNG_EXPORT void run_graphite(GrSegment* pSeg/*not NULL*/);
    GRNG_EXPORT void choose_silf(GrSegment* pSeg/*not NULL*/, uint32 script);
    GRNG_EXPORT unsigned int number_of_slots_in_segment(const GrSegment* pSeg/*not NULL*/);      //one slot per glyph
    GRNG_EXPORT const Slot* first_slot_in_segment(GrSegment* pSeg/*not NULL*/);
    
    GRNG_EXPORT int add_features(GrSegment* pSeg/*not NULL*/, const Features* feats);
}
  
  
}}}} // namespace
