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
    //in the following method, pError may be NULL. if it is not NULL, and the buffer contains some bad bytes not in the utf range, then *pError is set to point at the first bad byte. Otherwise, *pError is set to NULL.
    GRNG_EXPORT size_t count_unicode_characters(encform enc, const void* buffer_begin, const void* buffer_end/*don't go on or past end, If NULL then ignored*/, const void** pError);   //Also stops on nul. Any nul is not in the count
}


extern "C"
{
    GRNG_EXPORT GrSegment* make_seg(const GrFont* font, const GrFace* face, uint32 script, encform enc, const void* pStart, size_t nChars, int dir);
                      //When finished with the GrFont, call seg_destroy    
    GRNG_EXPORT GrSegment* make_seg_using_features(const GrFont* font, const GrFace* face, uint32 script, const Features* pFeats, encform enc, const void* pStart, size_t nChars, int dir);
                      //When finished with the GrFont, call seg_destroy    
    GRNG_EXPORT void seg_destroy(GrSegment* p);

    GRNG_EXPORT float seg_advance_X(const GrSegment* pSeg/*not NULL*/);
    GRNG_EXPORT float seg_advance_Y(const GrSegment* pSeg/*not NULL*/);
    GRNG_EXPORT unsigned int seg_n_cinfo(const GrSegment* pSeg/*not NULL*/);
    GRNG_EXPORT const CharInfo* seg_cinfo(const GrSegment* pSeg/*not NULL*/, unsigned int index/*must be <number_of_CharInfo*/);
    GRNG_EXPORT void seg_run_graphite(GrSegment* pSeg/*not NULL*/);
    GRNG_EXPORT void seg_choose_silf(GrSegment* pSeg/*not NULL*/, uint32 script);
    GRNG_EXPORT unsigned int seg_n_slots(const GrSegment* pSeg/*not NULL*/);      //one slot per glyph
    GRNG_EXPORT const Slot* seg_first_slot(GrSegment* pSeg/*not NULL*/);    //may give a base slot or a slot which is attached to another
    
    GRNG_EXPORT int seg_add_features(GrSegment* pSeg/*not NULL*/, const Features* feats);       //tbd what is return value?
}
  
  
}}}} // namespace
