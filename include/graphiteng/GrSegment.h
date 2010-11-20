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

namespace org { namespace sil { namespace graphite { namespace v2 {

class GrSegment;
class GrFace;
class CharInfo;
class Features;
class Slot;
class GrFont;

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
