/*  GRAPHITE2 LICENSING

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
#include "graphite2/Segment.h"
#include "processUTF.h"
#include "Segment.h"

using namespace graphite2;

namespace 
{
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

  static gr_segment* makeAndInitialize(const Font *font, const Face *face, uint32 script, const Features* pFeats/*must not be NULL*/, gr_encform enc, const void* pStart, size_t nChars, int dir)
  {
      // if (!font) return NULL;
      Segment* pRes=new Segment(nChars, face, script, dir);

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
      return static_cast<gr_segment*>(pRes);
  }

  
}


extern "C" {


size_t gr_count_unicode_characters(gr_encform enc, const void* buffer_begin, const void* buffer_end/*don't go on or past end, If NULL then ignored*/, const void** pError)   //Also stops on nul. Any nul is not in the count
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


gr_segment* gr_make_seg(const gr_font *font, const gr_face *face, gr_uint32 script, const gr_feature_val* pFeats/*must not be IsNull*/, gr_encform enc, const void* pStart, size_t nChars, int dir)
{
    if (pFeats == NULL)
        pFeats = static_cast<const gr_feature_val*>(face->theSill().cloneFeatures(0));
    return makeAndInitialize(font, face, script, pFeats, enc, pStart, nChars, dir);
}


void gr_seg_destroy(gr_segment* p)
{
    delete p;
}


float gr_seg_advance_X(const gr_segment* pSeg/*not NULL*/)
{
    assert(pSeg);
    return pSeg->advance().x;
}


float gr_seg_advance_Y(const gr_segment* pSeg/*not NULL*/)
{
    assert(pSeg);
    return pSeg->advance().y;
}


unsigned int gr_seg_n_cinfo(const gr_segment* pSeg/*not NULL*/)
{
    assert(pSeg);
    return pSeg->charInfoCount();
}


const gr_char_info* gr_seg_cinfo(const gr_segment* pSeg/*not NULL*/, unsigned int index/*must be <number_of_CharInfo*/)
{
    assert(pSeg);
    return static_cast<const gr_char_info*>(pSeg->charinfo(index));
}

unsigned int gr_seg_n_slots(const gr_segment* pSeg/*not NULL*/)
{
    assert(pSeg);
    return pSeg->slotCount();
}

const gr_slot* gr_seg_first_slot(gr_segment* pSeg/*not NULL*/)
{
    assert(pSeg);
    return static_cast<const gr_slot*>(pSeg->first());
}

const gr_slot* gr_seg_last_slot(gr_segment* pSeg/*not NULL*/)
{
    assert(pSeg);
    return static_cast<const gr_slot*>(pSeg->last());
}

void gr_seg_char_slots(const gr_segment *pSeg, gr_uint32 *begins, gr_uint32 *ends, gr_slot **sbegins, gr_slot **sends)
{
    assert(pSeg && begins && ends);
    pSeg->getCharSlots(begins, ends, reinterpret_cast<Slot**>(sbegins), reinterpret_cast<Slot**>(sends));
}


} // extern "C"
