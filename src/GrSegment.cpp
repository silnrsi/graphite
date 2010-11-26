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
GRNG_EXPORT size_t count_unicode_characters(encform enc, const void* buffer_begin, const void* buffer_end/*don't go on or past end, If NULL then ignored*/, const void** pError)   //Also stops on nul. Any nul is not in the count
{
  if (buffer_end)
  {
    BufferLimit limit(enc, buffer_begin, buffer_end);
    CharCounterToNul counter;
    return doCountUnicodeCharacters(limit, &counter, pError);
  }
  else
  {
    NoLimit limit(enc, buffer_begin);
    CharCounterToNul counter;
    return doCountUnicodeCharacters(limit, &counter, pError);
  }
}


GRNG_EXPORT GrSegment* make_seg(const GrFont *font, const GrFace *face, uint32 script, encform enc, const void* pStart, size_t nChars, int dir)
{
    return makeAndInitialize(font, face, script, face->theSill().cloneFeatures(0/*0 means default*/), enc, pStart, nChars, dir);
}


GRNG_EXPORT GrSegment* make_seg_using_features(const GrFont *font, const GrFace *face, uint32 script, const Features* pFeats/*must not be IsNull*/, encform enc, const void* pStart, size_t nChars, int dir)
{
    return makeAndInitialize(font, face, script, pFeats, enc, pStart, nChars, dir);
}


GRNG_EXPORT void destroy_seg(GrSegment* p)
{
    delete p;
}


GRNG_EXPORT float seg_advance_X(const GrSegment* pSeg/*not NULL*/)
{
    assert(pSeg);
    return pSeg->advance().x;
}


GRNG_EXPORT float seg_advance_Y(const GrSegment* pSeg/*not NULL*/)
{
    assert(pSeg);
    return pSeg->advance().y;
}


GRNG_EXPORT unsigned int seg_n_cinfo(const GrSegment* pSeg/*not NULL*/)
{
    assert(pSeg);
    return pSeg->charInfoCount();
}


GRNG_EXPORT const CharInfo* seg_cinfo(const GrSegment* pSeg/*not NULL*/, unsigned int index/*must be <number_of_CharInfo*/)
{
    assert(pSeg);
    return pSeg->charinfo(index);
}

GRNG_EXPORT void seg_run_graphite(GrSegment* pSeg/*not NULL*/)
{
    assert(pSeg);
    return pSeg->runGraphite();
}


GRNG_EXPORT void seg_choose_silf(GrSegment* pSeg/*not NULL*/, uint32 script)
{
    assert(pSeg);
    return pSeg->chooseSilf(script);
}


GRNG_EXPORT unsigned int seg_n_slots(const GrSegment* pSeg/*not NULL*/)
{
    assert(pSeg);
    return pSeg->slotCount();
}


GRNG_EXPORT const Slot* seg_first_slot(GrSegment* pSeg/*not NULL*/)
{
    assert(pSeg);
    return pSeg->first();
}


GRNG_EXPORT int seg_add_features(GrSegment* pSeg/*not NULL*/, const Features* feats)
{
    if (!feats)
    return -2;      //the smallest value that can normally be returned is -1
    assert(pSeg);
    return pSeg->addFeatures(*feats);
}

}

}}}} // namespace