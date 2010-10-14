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
#include "graphiteng/GrSegment.h"
#include "graphiteng/Slot.h"
#include "GrSegmentImp.h"
#include "processUTF.h"

namespace org { namespace sil { namespace graphite { namespace v2 {

class CharCounter
{
public:
      CharCounter()
      :	  m_nCharsProcessed(0) 
      {
      }	  

      bool processChar(uint32 cid/*unicode character*/)		//return value indicates if should stop processing
      {
	  ++m_nCharsProcessed;
	  return true;
      }

      size_t charsProcessed() const { return m_nCharsProcessed; }

private:
      size_t m_nCharsProcessed ;
};


template <class LIMIT, class CHARPROCESSOR>
size_t doCountUnicodeCharacters(const LIMIT& limit, CHARPROCESSOR* pProcessor, const void** pError)
{
    BreakOnError breakOnError;
    
    processUTF(limit/*when to stop processing*/, pProcessor, &breakOnError);
    if (pError) {
        *pError = breakOnError.m_pErrorPos;
    }        
    return pProcessor->charsProcessed();
}    

class CharCounterToNul
{
public:
      CharCounterToNul()
      :	  m_nCharsProcessed(0) 
      {
      }	  

      bool processChar(uint32 cid/*unicode character*/)		//return value indicates if should stop processing
      {
	  if (cid==0)
	      return false;
	  ++m_nCharsProcessed;
	  return true;
      }

      size_t charsProcessed() const { return m_nCharsProcessed; }

private:
      size_t m_nCharsProcessed ;
};

static GrSegment* makeAndInitialize(const GrFont *font, const GrFace *face, uint32 script, const Features* pFeats/*must not be NULL*/, encform enc, const void* pStart, size_t nChars, int dir)
{
    if (!font) return NULL;
    GrSegment* pRes=new GrSegment(nChars, face, script, dir);

    pRes->read_text(face, pFeats, enc, pStart, nChars);
    pRes->runGraphite();
    // run the line break passes
    // run the substitution passes
    pRes->prepare_pos(font);
    // run the positioning passes
    pRes->finalise(font);
#ifndef DISABLE_TRACING
    pRes->logSegment(enc, pStart, nChars);
#endif
    return pRes;
}


extern "C" 
{
GRNG_EXPORT size_t count_unicode_characters(encform enc, const void* buffer_begin, const void* buffer_end/*as in stl i.e. don't use end*/, const void** pError)
{
    BufferLimit limit(enc, buffer_begin, buffer_end);
    CharCounter counter;
    return doCountUnicodeCharacters(limit, &counter, pError);
}


GRNG_EXPORT size_t count_unicode_characters_with_max_count(encform enc, const void* buffer_begin, const void* buffer_end/*as in stl i.e. don't use end*/, size_t maxCount, const void** pError)
{
    BufferAndCharacterCountLimit limit(enc, buffer_begin, buffer_end, maxCount);
    CharCounter counter;
    return doCountUnicodeCharacters(limit, &counter, pError);
}


GRNG_EXPORT size_t count_unicode_characters_to_nul(encform enc, const void* buffer_begin, const void** pError)	//the nul is not in the count
{
    NoLimit limit(enc, buffer_begin);
    CharCounterToNul counter;
    return doCountUnicodeCharacters(limit, &counter, pError);
}


GRNG_EXPORT size_t count_unicode_characters_to_nul_or_end(encform enc, const void* buffer_begin, const void* buffer_end/*don't go past end*/, const void** pError)	//the nul is not in the count
{
    BufferLimit limit(enc, buffer_begin, buffer_end);
    CharCounterToNul counter;
    return doCountUnicodeCharacters(limit, &counter, pError);
}


GRNG_EXPORT size_t count_unicode_characters_to_nul_or_end_with_max_count(encform enc, const void* buffer_begin, const void* buffer_end/*don't go past end*/, size_t maxCount, const void** pError)	//the nul is not in the count
{
    BufferAndCharacterCountLimit limit(enc, buffer_begin, buffer_end, maxCount);
    CharCounterToNul counter;
    return doCountUnicodeCharacters(limit, &counter, pError);
}


GRNG_EXPORT GrSegment* make_GrSegment(const GrFont *font, const GrFace *face, uint32 script, encform enc, const void* pStart, size_t nChars, int dir)
{
    return makeAndInitialize(font, face, script, face->theFeatures().cloneFeatures(0/*0 means default*/), enc, pStart, nChars, dir);
}


GRNG_EXPORT GrSegment* make_GrSegment_using_features(const GrFont *font, const GrFace *face, uint32 script, const Features* pFeats/*must not be IsNull*/, encform enc, const void* pStart, size_t nChars, int dir)
{
    return makeAndInitialize(font, face, script, pFeats, enc, pStart, nChars, dir);
}


GRNG_EXPORT void destroy_GrSegment(GrSegment* p)
{
    delete p;
}


GRNG_EXPORT float advance_X(const GrSegment* pSeg/*not NULL*/)
{
    return pSeg->advance().x;
}


GRNG_EXPORT float advance_Y(const GrSegment* pSeg/*not NULL*/)
{
    return pSeg->advance().y;
}


GRNG_EXPORT unsigned int number_of_CharInfo(const GrSegment* pSeg/*not NULL*/)
{
    return pSeg->charInfoCount();
}


GRNG_EXPORT const CharInfo* charInfo(const GrSegment* pSeg/*not NULL*/, unsigned int index/*must be <number_of_CharInfo*/)
{
    return pSeg->charinfo(index);
}

GRNG_EXPORT void runGraphite(GrSegment* pSeg/*not NULL*/)
{
    return pSeg->runGraphite();
}


GRNG_EXPORT void chooseSilf(GrSegment* pSeg/*not NULL*/, uint32 script)
{
    return pSeg->chooseSilf(script);
}


GRNG_EXPORT unsigned int number_of_slots_in_segment(const GrSegment* pSeg/*not NULL*/)
{
    return pSeg->slotCount();
}


GRNG_EXPORT const Slot* first_slot_in_segment(GrSegment* pSeg/*not NULL*/)
{
    return pSeg->first();
}


GRNG_EXPORT int addFeatures(GrSegment* pSeg/*not NULL*/, const Features* feats)
{
    if (!feats)
    return -2;      //the smallest value that can normally be returned is -1
    
    return pSeg->addFeatures(*feats);
}

}

}}}} // namespace
