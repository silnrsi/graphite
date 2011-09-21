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
    If not, write to the Free Software Foundation, 51 Franklin Street, 
    Suite 500, Boston, MA 02110-1335, USA or visit their web page on the 
    internet at http://www.fsf.org/licenses/lgpl.html.

Alternatively, the contents of this file may be used under the terms of the
Mozilla Public License (http://mozilla.org/MPL) or the GNU General Public
License, as published by the Free Software Foundation, either version 2
of the License or (at your option) any later version.
*/
#pragma once 

#include <iterator>
#include "Main.h"
#include "graphite2/Segment.h"

namespace graphite2 {

class BufferLimit
{
public:
    BufferLimit(gr_encform enc2, const void* pStart2, const void* pEnd/*as in stl i.e. don't use end*/) : m_enc(enc2), m_pStart(pStart2) {
	size_t nFullTokens = (static_cast<const char*>(pEnd)-static_cast<const char *>(m_pStart))/int(m_enc); //rounds off partial tokens
	m_pEnd = static_cast<const char *>(m_pStart) + (nFullTokens*int(m_enc));
    }
    gr_encform enc() const { return m_enc; }
    const void* pStart() const { return m_pStart; }

    bool inBuffer (const void* pCharLastSurrogatePart, uint32 /*val*/) const { return pCharLastSurrogatePart<m_pEnd; }	//also called on charstart by needMoreChars()

    bool needMoreChars (const void* pCharStart, size_t /*nProcessed*/) const { return inBuffer(pCharStart, 1); }

private:
    const void* m_pEnd;
    gr_encform m_enc;
    const void* m_pStart;
};


class IgnoreErrors
{
public:
    //for all of the ignore* methods is the parameter is false, the return result must be true
    static bool ignoreUnicodeOutOfRangeErrors(bool /*isBad*/) { return true; }
    static bool ignoreBadSurrogatesErrors(bool /*isBad*/) { return true; }

    static bool handleError(const void* /*pPositionOfError*/) { return true;}
};

inline unsigned int utf8_extrabytes(const unsigned int topNibble) { return (0xE5FF0000>>(2*topNibble))&0x3; }

inline unsigned int utf8_mask(const unsigned int seq_extra) { return ((0xFEC0>>(4*seq_extra))&0xF)<<4; }

class Utf8Consumer
{
public:
    Utf8Consumer(const uint8* pCharStart2) : m_pCharStart(pCharStart2) {}

    const uint8* pCharStart() const { return m_pCharStart; }

private:
    template <class ERRORHANDLER>
    bool respondToError(uint32* pRes, ERRORHANDLER* pErrHandler) {       //return value is if should stop parsing
        *pRes = 0xFFFD;
        if (!pErrHandler->handleError(m_pCharStart)) {
            return false;
        }
        ++m_pCharStart;
        return true;
    }

public:
    template <class LIMIT, class ERRORHANDLER>
    inline bool consumeChar(const LIMIT& limit, uint32* pRes, ERRORHANDLER* pErrHandler) {			//At start, limit.inBuffer(m_pCharStart) is true. return value is iff character contents does not go past limit
        const unsigned int seq_extra = utf8_extrabytes(*m_pCharStart >> 4);        //length of sequence including *m_pCharStart is 1+seq_extra
        if (!limit.inBuffer(m_pCharStart+(seq_extra), *m_pCharStart)) {
            return false;
        }

        *pRes = *m_pCharStart ^ utf8_mask(seq_extra);

        if (seq_extra) {
            switch(seq_extra) {    //hopefully the optimizer will implement this as a jump table. If not the above if should cover the majority case.
                case 3: {
                    if (pErrHandler->ignoreUnicodeOutOfRangeErrors(*m_pCharStart>=0xF8)) {		//the good case
                        ++m_pCharStart;
                        if (!pErrHandler->ignoreBadSurrogatesErrors((*m_pCharStart&0xC0)!=0x80)) {
                            return respondToError(pRes, pErrHandler);
                        }

                        *pRes <<= 6; *pRes |= *m_pCharStart & 0x3F;		//drop through
                    }
                    else {
                        return respondToError(pRes, pErrHandler);
                    }
                }
                case 2: {
                    ++m_pCharStart;
                    if (!pErrHandler->ignoreBadSurrogatesErrors((*m_pCharStart&0xC0)!=0x80)) {
                        return respondToError(pRes, pErrHandler);
                    }
                }
                *pRes <<= 6; *pRes |= *m_pCharStart & 0x3F;       //drop through
                case 1: {
                    ++m_pCharStart;
                    if (!pErrHandler->ignoreBadSurrogatesErrors((*m_pCharStart&0xC0)!=0x80)) {
                        return respondToError(pRes, pErrHandler);
                    }
                }
                *pRes <<= 6; *pRes |= *m_pCharStart & 0x3F;
             }
        }
        ++m_pCharStart;
        return true;
    }

private:
    const uint8 *m_pCharStart;
};



class Utf16Consumer
{
public:
      Utf16Consumer(const uint16* pCharStart2) : m_pCharStart(pCharStart2) {}

      const uint16* pCharStart() const { return m_pCharStart; }

private:
    template <class ERRORHANDLER>
    bool respondToError(uint32* pRes, ERRORHANDLER* pErrHandler) {       //return value is if should stop parsing
        *pRes = 0xFFFD;
        if (!pErrHandler->handleError(m_pCharStart)) {
            return false;
        }
        ++m_pCharStart;
        return true;
    }

public:
      template <class LIMIT, class ERRORHANDLER>
      inline bool consumeChar(const LIMIT& limit, uint32* pRes, ERRORHANDLER* pErrHandler)			//At start, limit.inBuffer(m_pCharStart) is true. return value is iff character contents does not go past limit
      {
	  *pRes = *m_pCharStart;
      if (0xD800 > *pRes || !pErrHandler->ignoreUnicodeOutOfRangeErrors(*pRes >= 0xE000)) {
          ++m_pCharStart;
          return true;
      }

      if (!pErrHandler->ignoreBadSurrogatesErrors(*pRes >= 0xDC00)) {        //second surrogate is incorrectly coming first
          return respondToError(pRes, pErrHandler);
      }

      ++m_pCharStart;
	  if (!limit.inBuffer(m_pCharStart, *pRes)) {
	      return false;
	  }

	  uint32 ul = *(m_pCharStart);
	  if (!pErrHandler->ignoreBadSurrogatesErrors(0xDC00 > ul || ul > 0xDFFF)) {
          return respondToError(pRes, pErrHandler);
	  }
	  ++m_pCharStart;
	  *pRes =  ((*pRes - 0xD800)<<10) + ul - 0xDC00;
	  return true;
      }

private:
      const uint16 *m_pCharStart;
};


class Utf32Consumer
{
public:
      Utf32Consumer(const uint32* pCharStart2) : m_pCharStart(pCharStart2) {}

      const uint32* pCharStart() const { return m_pCharStart; }

private:
    template <class ERRORHANDLER>
    bool respondToError(uint32* pRes, ERRORHANDLER* pErrHandler) {       //return value is if should stop parsing
        *pRes = 0xFFFD;
        if (!pErrHandler->handleError(m_pCharStart)) {
            return false;
        }
        ++m_pCharStart;
        return true;
    }

public:
      template <class LIMIT, class ERRORHANDLER>
      inline bool consumeChar(const LIMIT& limit, uint32* pRes, ERRORHANDLER* pErrHandler)			//At start, limit.inBuffer(m_pCharStart) is true. return value is iff character contents does not go past limit
      {
	  *pRes = *m_pCharStart;
      if (pErrHandler->ignoreUnicodeOutOfRangeErrors(!(*pRes<0xD800 || (*pRes>=0xE000 && *pRes<0x110000)))) {
          if (!limit.inBuffer(++m_pCharStart, *pRes))
            return false;
          else
            return true;
      }

      return respondToError(pRes, pErrHandler);
      }

private:
      const uint32 *m_pCharStart;
};





template <class LIMIT, class CHARPROCESSOR, class ERRORHANDLER>
void processUTF(const LIMIT& limit/*when to stop processing*/, CHARPROCESSOR* pProcessor, ERRORHANDLER* pErrHandler)
{
     uint32             cid;
     switch (limit.enc()) {
       case gr_utf8 : {
        const uint8 *pInit = static_cast<const uint8 *>(limit.pStart());
	    Utf8Consumer consumer(pInit);
        for (;limit.needMoreChars(consumer.pCharStart(), pProcessor->charsProcessed());) {
            const uint8 *pCur = consumer.pCharStart();
		    if (!consumer.consumeChar(limit, &cid, pErrHandler))
		        break;
		    if (!pProcessor->processChar(cid, pCur - pInit))
		        break;
        }
        break; }
       case gr_utf16: {
        const uint16* pInit = static_cast<const uint16 *>(limit.pStart());
        Utf16Consumer consumer(pInit);
        for (;limit.needMoreChars(consumer.pCharStart(), pProcessor->charsProcessed());) {
            const uint16 *pCur = consumer.pCharStart();
    		if (!consumer.consumeChar(limit, &cid, pErrHandler))
	    	    break;
		    if (!pProcessor->processChar(cid, pCur - pInit))
		        break;
            }
	    break;
        }
       case gr_utf32 : default: {
        const uint32 *pInit = static_cast<const uint32 *>(limit.pStart());
	    Utf32Consumer consumer(pInit);
        for (;limit.needMoreChars(consumer.pCharStart(), pProcessor->charsProcessed());) {
            const uint32 *pCur = consumer.pCharStart();
		    if (!consumer.consumeChar(limit, &cid, pErrHandler))
		        break;
		    if (!pProcessor->processChar(cid, pCur - pInit))
		        break;
            }
        break;
        }
    }
}

    class ToUtf8Processor
    {
    public:
        // buffer length should be three times the utf16 length or
        // four times the utf32 length to cover the worst case
        ToUtf8Processor(uint8 * buffer, size_t maxLength) :
            m_count(0), m_byteLength(0), m_maxLength(maxLength), m_buffer(buffer)
        {}
        bool processChar(uint32 cid, size_t /*offset*/)
        {
            // taken from Unicode Book ch3.9
            if (cid <= 0x7F)
                m_buffer[m_byteLength++] = cid;
            else if (cid <= 0x07FF)
            {
                if (m_byteLength + 2 >= m_maxLength)
                    return false;
                m_buffer[m_byteLength++] = 0xC0 + (cid >> 6);
                m_buffer[m_byteLength++] = 0x80 + (cid & 0x3F);
            }
            else if (cid <= 0xFFFF)
            {
                if (m_byteLength + 3 >= m_maxLength)
                    return false;
                m_buffer[m_byteLength++] = 0xE0 + (cid >> 12);
                m_buffer[m_byteLength++] = 0x80 + ((cid & 0x0FC0) >> 6);
                m_buffer[m_byteLength++] = 0x80 +  (cid & 0x003F);
            }
            else if (cid <= 0x10FFFF)
            {
                if (m_byteLength + 4 >= m_maxLength)
                    return false;
                m_buffer[m_byteLength++] = 0xF0 + (cid >> 18);
                m_buffer[m_byteLength++] = 0x80 + ((cid & 0x3F000) >> 12);
                m_buffer[m_byteLength++] = 0x80 + ((cid & 0x00FC0) >> 6);
                m_buffer[m_byteLength++] = 0x80 +  (cid & 0x0003F);
            }
            else
            {
                // ignore
            }
            m_count++;
            if (m_byteLength >= m_maxLength)
                return false;
            return true;
        }
        size_t charsProcessed() const { return m_count; }
        size_t bytesProcessed() const { return m_byteLength; }
    private:
        size_t m_count;
        size_t m_byteLength;
        size_t m_maxLength;
        uint8 * m_buffer;
    };
//
//    class ToUtf16Processor
//    {
//    public:
//        // buffer length should be twice the utf32 length
//        // to cover the worst case
//        ToUtf16Processor(uint16 * buffer, size_t maxLength) :
//            m_count(0), m_uint16Length(0), m_maxLength(maxLength), m_buffer(buffer)
//        {}
//        bool processChar(uint32 cid, size_t /*offset*/)
//        {
//            // taken from Unicode Book ch3.9
//            if (cid <= 0xD800)
//                m_buffer[m_uint16Length++] = cid;
//            else if (cid < 0xE000)
//            {
//                // skip for now
//            }
//            else if (cid >= 0xE000 && cid <= 0xFFFF)
//                m_buffer[m_uint16Length++] = cid;
//            else if (cid <= 0x10FFFF)
//            {
//                if (m_uint16Length + 2 >= m_maxLength)
//                    return false;
//                m_buffer[m_uint16Length++] = 0xD800 + ((cid & 0xFC00) >> 10) + ((cid >> 16) - 1);
//                m_buffer[m_uint16Length++] = 0xDC00 + ((cid & 0x03FF) >> 12);
//            }
//            else
//            {
//                // ignore
//            }
//            m_count++;
//            if (m_uint16Length == m_maxLength)
//                return false;
//            return true;
//        }
//        size_t charsProcessed() const { return m_count; }
//        size_t uint16Processed() const { return m_uint16Length; }
//    private:
//        size_t m_count;
//        size_t m_uint16Length;
//        size_t m_maxLength;
//        uint16 * m_buffer;
//    };
//
    class ToUtf32Processor
    {
    public:
        ToUtf32Processor(uint32 * buffer, size_t maxLength) :
            m_count(0), m_maxLength(maxLength), m_buffer(buffer) {}
        bool processChar(uint32 cid, size_t /*offset*/)
        {
            m_buffer[m_count++] = cid;
            if (m_count == m_maxLength)
                return false;
            return true;
        }
        size_t charsProcessed() const { return m_count; }
    private:
        size_t m_count;
        size_t m_maxLength;
        uint32 * m_buffer;
    };



	class utf32_iterator : public std::iterator<std::input_iterator_tag, uint32>
	{
		static const uint32 limit = 0x110000;

	public:
		typedef uint32	codepoint_type;

		utf32_iterator(const void * us=0) : _p(reinterpret_cast<const uint32 *>(us)) {}

		utf32_iterator & operator ++ () 		{ ++_p; return *this; }
		utf32_iterator   operator ++ (int) 	{ utf32_iterator tmp(*this); operator++(); return tmp; }

		bool operator == (const utf32_iterator & rhs) { return _p == rhs._p; }
		bool operator != (const utf32_iterator & rhs) { return !operator==(rhs); }

		value_type 			operator * () const { return *operator->(); }
		const value_type  * operator ->() const {
			static const uint32	replacement_char = 0xFFFD;
			return *_p < limit ? _p : &replacement_char;
		}

		operator const uint32 * () const { return _p; }
	protected:
		const codepoint_type * _p;
	};


	class utf16_iterator : public std::iterator<std::input_iterator_tag, uint32>
	{
		static const uint32	replacement_char = 0xFFFD;
		static const int	surrogate_offset = 0x10000 - (0xD800 << 10) - 0xDC00;
	public:
		typedef uint16	codepoint_type;

		utf16_iterator(const void * us=0) : _p(reinterpret_cast<const codepoint_type *>(us)) {}

		utf16_iterator & operator ++ () 		{ ++_p; return *this; }
		utf16_iterator   operator ++ (int) 		{ utf16_iterator tmp(*this); operator++(); return tmp; }

		bool operator == (const utf16_iterator & rhs) { return _p >= rhs._p; }
		bool operator != (const utf16_iterator & rhs) { return !operator==(rhs); }

		value_type 			operator * () const {
			const uint32	uh = *_p;

			if (0xD800 > uh || uh > 0xDBFF)	{ _v = uh; return _v; }
			const uint32 ul = *++_p;
			if (0xDC00 > ul || ul > 0xDFFF) { --_p; _v = replacement_char; return _v; }
			_v = (uh<<10) + ul - surrogate_offset;
			return _v;
		}

		const value_type *	operator ->() const { operator * (); return &_v; }

		operator const codepoint_type * () const { return _p; }
	protected:
		mutable const 	codepoint_type  *_p;
		mutable 		value_type 		 _v;
	};


	class utf8_iterator : public std::iterator<std::input_iterator_tag, uint32>
	{
		static const uint32	replacement_char = 0xFFFD;
	public:
		typedef uint8	codepoint_type;

		utf8_iterator(const void * us=0) : _p(reinterpret_cast<const codepoint_type *>(us)) {}

		utf8_iterator & operator ++ () 		{ ++_p; return *this; }
		utf8_iterator   operator ++ (int) 	{ utf8_iterator tmp(*this); operator++(); return tmp; }

		bool operator == (const utf8_iterator & rhs) { return _p >= rhs._p; }
		bool operator != (const utf8_iterator & rhs) { return !operator==(rhs); }

		value_type 			operator * () const {
			static const int8 utf8_sz_lut[16] = {1,1,1,1,1,1,1,1,	// 1 byte
											   	 0,0,0,0,  			// trailing byte
											   	 2,2,				// 2 bytes
											   	 3,					// 3 bytes
											   	 4};				// 4 bytes
			static const byte utf8_mask_lut[5] = {0x80,0x00,0xC0,0xE0,0xF0};

			const int8    	seq_sz = utf8_sz_lut[*_p >> 4];
		    _v = *_p ^ utf8_mask_lut[seq_sz];

		    switch(seq_sz) {
		        case 4:     _v <<= 6; _v |= *++_p & 0x7F;
		        case 3:     _v <<= 6; _v |= *++_p & 0x7F;
		        case 2:     _v <<= 6; _v |= *++_p & 0x7F; break;
		        case 1:     break;
		        case 0:     _v = replacement_char; break;
		    }
		    return _v;
		}

		const value_type *	operator ->() const { operator * (); return &_v; }

		operator const codepoint_type * () const { return _p; }
	protected:
		mutable const 	codepoint_type  *_p;
		mutable 		value_type 		 _v;
	};

} // namespace graphite2
