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
#include "graphiteng/SegmentHandle.h"
#include "graphiteng/SlotHandle.h"
#include "GrSegment.h"
#include "processUTF.h"

namespace org { namespace sil { namespace graphite { namespace v2 {

GRNG_EXPORT void DeleteSegment(GrSegment *p)
{
    delete p;
}


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

/*static*/ size_t SegmentHandle::countUnicodeCharacters(SegmentHandle::encform enc, const void* buffer_begin, const void* buffer_end/*as in stl i.e. don't use end*/, const void** pError)
{
    BufferLimit limit(enc, buffer_begin, buffer_end);
    CharCounter counter;
    return doCountUnicodeCharacters(limit, &counter, pError);
}


/*static*/ size_t SegmentHandle::countUnicodeCharacters(SegmentHandle::encform enc, const void* buffer_begin, const void* buffer_end/*as in stl i.e. don't use end*/, size_t maxCount, const void** pError)
{
    BufferAndCharacterCountLimit limit(enc, buffer_begin, buffer_end, maxCount);
    CharCounter counter;
    return doCountUnicodeCharacters(limit, &counter, pError);
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


/*static*/ size_t SegmentHandle::countUnicodeCharactersToNul(SegmentHandle::encform enc, const void* buffer_begin, const void** pError)	//the nul is not in the count
{
    NoLimit limit(enc, buffer_begin);
    CharCounterToNul counter;
    return doCountUnicodeCharacters(limit, &counter, pError);
}


/*static*/ size_t SegmentHandle::countUnicodeCharactersToNul(SegmentHandle::encform enc, const void* buffer_begin, const void* buffer_end/*don't go past end*/, const void** pError)	//the nul is not in the count
{
    BufferLimit limit(enc, buffer_begin, buffer_end);
    CharCounterToNul counter;
    return doCountUnicodeCharacters(limit, &counter, pError);
}


/*static*/ size_t SegmentHandle::countUnicodeCharactersToNul(SegmentHandle::encform enc, const void* buffer_begin, const void* buffer_end/*don't go past end*/, size_t maxCount, const void** pError)	//the nul is not in the count
{
    BufferAndCharacterCountLimit limit(enc, buffer_begin, buffer_end, maxCount);
    CharCounterToNul counter;
    return doCountUnicodeCharacters(limit, &counter, pError);
}



SegmentHandle::SegmentHandle(const GrFont *font, const GrFace *face, uint32 script, encform enc, const void* pStart, size_t nChars, int dir)
{
    initialize(font, face, script, face->theFeatures().cloneFeatures(0/*0 means default*/), enc, pStart, nChars, dir);
}


SegmentHandle::SegmentHandle(const GrFont *font, const GrFace *face, uint32 script, const FeaturesHandle& pFeats/*must not be IsNull*/, encform enc, const void* pStart, size_t nChars, int dir)
{
    initialize(font, face, script, pFeats, enc, pStart, nChars, dir);
}


int SegmentHandle::length() const
{
    return ptr()->length();
}


float SegmentHandle::advanceX() const
{
    return ptr()->advance().x;
}


float SegmentHandle::advanceY() const
{
    return ptr()->advance().y;
}


CharInfo *SegmentHandle::charInfo(int index) const
{
    return ptr()->charinfo(index);
}

void SegmentHandle::runGraphite() const
{
    return ptr()->runGraphite();
}


void SegmentHandle::chooseSilf(uint32 script) const
{
    return ptr()->chooseSilf(script);
}


int SegmentHandle::addFeatures(const FeaturesHandle& feats) const
{
    if (feats.isNull())
	return -2;		//the smallest value that can normally be returned is -1
    
    return ptr()->addFeatures(*feats.ptr());
}

SlotHandle SegmentHandle::first()
{
    return SlotHandle(ptr()->first());
}

void SegmentHandle::initialize(const GrFont *font, const GrFace *face, uint32 script, const FeaturesHandle& pFeats/*must not be IsNull*/, encform enc, const void* pStart, size_t nChars, int dir)
{
    setPtr(new GrSegment(nChars, face, script, dir));

    ptr()->read_text(face, pFeats, enc, pStart, nChars);
    ptr()->runGraphite();
    // run the line break passes
    // run the substitution passes
    ptr()->prepare_pos(font);
    // run the positioning passes
    ptr()->finalise(font);
#ifndef DISABLE_TRACING
    ptr()->logSegment(enc, pStart, nChars);
#endif
}

}}}} // namespace
