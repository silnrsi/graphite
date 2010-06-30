#ifndef PROCESS_UTF_INCLUDE
#define PROCESS_UTF_INCLUDE

#include "graphiteng/Types.h"
#include "graphiteng/SegmentHandle.h"



class NoLimit		//relies on the processor.processChar() failing, such as because of a terminating nul character
{
public:
    NoLimit(SegmentHandle::encform enc2, const void* pStart2) : m_enc(enc2), m_pStart(pStart2) {}
    SegmentHandle::encform enc() const { return m_enc; }
    const void* pStart() const { return m_pStart; }

    static bool inBuffer(const void* pCharLastSurrogatePart) { return true; }
    static bool needMoreChars(const void* pCharStart, size_t nProcessed) { return true; }
    
private:
    SegmentHandle::encform m_enc;
    const void* m_pStart;
};


class CharacterCountLimit
{
public:
    CharacterCountLimit(SegmentHandle::encform enc2, const void* pStart2, size_t numchars) : m_numchars(numchars), m_enc(enc2), m_pStart(pStart2) {}
    SegmentHandle::encform enc() const { return m_enc; }
    const void* pStart() const { return m_pStart; }

    static bool inBuffer(const void* pCharLastSurrogatePart) { return true; }
    bool needMoreChars(const void* pCharStart, size_t nProcessed) const { return nProcessed<m_numchars; }
    
private:
    size_t m_numchars;
    SegmentHandle::encform m_enc;
    const void* m_pStart;
};


class BufferLimit
{
public:
    BufferLimit(SegmentHandle::encform enc2, const void* pStart2, const void* pEnd/*as in stl i.e. don't use end*/) : m_enc(enc2), m_pStart(pStart2) {
	size_t nFullTokens = (static_cast<const char*>(pEnd)-static_cast<const char *>(m_pStart))/int(m_enc); //rounds off partial tokens
	m_pEnd = static_cast<const char *>(m_pStart) + (nFullTokens*int(m_enc));
    }
    SegmentHandle::encform enc() const { return m_enc; }
    const void* pStart() const { return m_pStart; }
  
    bool inBuffer(const void* pCharLastSurrogatePart) const { return pCharLastSurrogatePart<m_pEnd; }	//also called on charstart by needMoreChars()

    bool needMoreChars(const void* pCharStart, size_t nProcessed) const { return inBuffer(pCharStart); }
     
private:
    const void* m_pEnd;
    SegmentHandle::encform m_enc;
    const void* m_pStart;
};


class BufferAndCharacterCountLimit : public BufferLimit
{
public:
    BufferAndCharacterCountLimit(SegmentHandle::encform enc2, const void* pStart2, const void* pEnd/*as in stl i.e. don't use end*/, size_t numchars) : BufferLimit(enc2, pStart2, pEnd), m_numchars(numchars) {}
  
    //inBuffer is inherited for convenience 
    bool needMoreChars(const void* pCharStart, size_t nProcessed) const { return nProcessed<m_numchars && inBuffer(pCharStart); }

private:
    size_t m_numchars;
};


const int utf8_sz_lut[16] = {1,1,1,1,1,1,1,1,        // 1 byte
                                          4,4,4,4,  // errors since trailing byte, catch later
                                          2,2,            // 2 bytes
                                          3,                 // 3 bytes
                                          4};                // 4 bytes

const byte utf8_mask_lut[5] = {0x00,0x00,0xC0,0xE0,0xF0};

class Utf8Consumer
{
public:
      Utf8Consumer(const uint8* pCharStart2) : m_pCharStart(pCharStart2) {}
      
      const uint8* pCharStart() const { return m_pCharStart; }
  
      template <class LIMIT>
      inline bool consumeChar(const LIMIT& limit, uint32* pRes)			//At start, limit.inBuffer(m_pCharStart) is true. return value is iff character contents does not go past limit
      {
	const size_t    seq_sz = utf8_sz_lut[*m_pCharStart >> 4];
	if (!limit.inBuffer(m_pCharStart+(seq_sz-1))) {
	    return false;
	}
	
	*pRes = *m_pCharStart ^ utf8_mask_lut[seq_sz];
	
	switch(seq_sz) {      
	    case 4:    
	    {	
		if ((*m_pCharStart>>3)==0x1E) {		//the good case
		    *pRes <<= 6; *pRes |= *++m_pCharStart & 0x3F;		//drop through
		else {
		    *pRes = 0xFFFD;
		    ++m_pCharStart; 
		    return true;			//this is an error. But carry on anyway?
		}		    
	    }
	    case 3:     *pRes <<= 6; *pRes |= *++m_pCharStart & 0x3F;
	    case 2:     *pRes <<= 6; *pRes |= *++m_pCharStart & 0x3F; break;
	    case 1: default:    break;
	}
	++m_pCharStart; 
	return true;
      }	
  
private:
      const uint8 *m_pCharStart;
};



class Utf16Consumer
{
private:
    static const unsigned int SURROGATE_OFFSET = 0x10000 - (0xD800 << 10) - 0xDC00;

public:
      Utf16Consumer(const uint16* pCharStart2) : m_pCharStart(pCharStart2) {}
      
      const uint16* pCharStart() const { return m_pCharStart; }
  
      template <class LIMIT>
      inline bool consumeChar(const LIMIT& limit, uint32* pRes)			//At start, limit.inBuffer(m_pCharStart) is true. return value is iff character contents does not go past limit
      {
	  *pRes = *(m_pCharStart)++;
	  if (*pRes > 0xDBFF || 0xD800 > *pRes)
	      return true;

	  if (!limit.inBuffer(m_pCharStart+1)) {
	      return false;
	  }

	  uint32 ul = *(m_pCharStart++);
	  if (0xDC00 > ul || ul > 0xDFFF) {
	      *pRes = 0xFFFD;
	      return true; 			//this is an error. But carry on anyway?
	  }
	  *pRes =  (*pRes<<10) + ul - SURROGATE_OFFSET;
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
  
      template <class LIMIT>
      inline bool consumeChar(const LIMIT& limit, uint32* pRes)			//At start, limit.inBuffer(m_pCharStart) is true. return value is iff character contents does not go past limit
      {
	  *pRes = *(m_pCharStart++);
	  return true;
      }

private:
      const uint32 *m_pCharStart;
};




/* The following template function assumes that LIMIT and CHARPROCESSOR have the following methods and semantics:

class LIMIT
{
public:
    SegmentHandle::encform enc() const;		//which of the below overloads of inBuffer() and needMoreChars() are called
    const void* pStart() const;			//start of first character to process
  
    bool inBuffer(const uint8* pCharLastSurrogatePart) const;	//whether or not the input is considered to be in the range of the buffer.
    bool inBuffer(const uint16* pCharLastSurrogatePart) const;	//whether or not the input is considered to be in the range of the buffer.

    bool needMoreChars(const uint8* pCharStart, size_t nProcessed) const; //whether or not the input is considered to be in the range of the buffer, and sufficient characters have been processed.
    bool needMoreChars(const uint16* pCharStart, size_t nProcessed) const; //whether or not the input is considered to be in the range of the buffer, and sufficient characters have been processed.
    bool needMoreChars(const uint32* pCharStart, size_t nProcessed) const; //whether or not the input is considered to be in the range of the buffer, and sufficient characters have been processed.
};

class CHARPROCESSOR
{
public:
    bool processChar(uint32 cid);		//return value indicates if should stop processing
    size_t charsProcessed() const;	//number of characters processed. Usually starts from 0 and incremented by processChar(). Passed in to LIMIT::needMoreChars
};

Useful reusable examples of LIMIT are:
NoLimit		//relies on the CHARPROCESSOR.processChar() failing, such as because of a terminating nul character
CharacterCountLimit //doesn't care about where the input buffer may end, but limits the number of unicode characters processed.
BufferLimit	//processes how ever many characters there are until the buffer end. characters straggling the end are not processed.
BufferAndCharacterCountLimit //processes a maximum number of characters there are until the buffer end. characters straggling the end are not processed.
*/

template <class LIMIT, class CHARPROCESSOR>
void processUTF(const LIMIT& limit/*when to stop processing*/, CHARPROCESSOR* pProcessor)
{
     uint32             cid;
     switch (limit.enc()) {
       case SegmentHandle::kutf8 : {
	    Utf8Consumer consumer(static_cast<const uint8 *>(limit.pStart()));
            for (;limit.needMoreChars(consumer.pCharStart(), pProcessor->charsProcessed());) {
		if (!consumer.consumeChar(limit, &cid))
		    break;
		if (!pProcessor->processChar(cid))
		    break;
            }
            break;
        }
       case SegmentHandle::kutf16: {
            Utf16Consumer consumer(static_cast<const uint16 *>(limit.pStart()));
            for (;limit.needMoreChars(consumer.pCharStart(), pProcessor->charsProcessed());) {
		if (!consumer.consumeChar(limit, &cid))
		    break;
		if (!pProcessor->processChar(cid))
		    break;
            }
	    break;
        }
       case SegmentHandle::kutf32 : default: {
	    Utf32Consumer consumer(static_cast<const uint32 *>(limit.pStart()));
            for (;limit.needMoreChars(consumer.pCharStart(), pProcessor->charsProcessed());) {
		if (!consumer.consumeChar(limit, &cid))
		    break;
		if (!pProcessor->processChar(cid))
		    break;
            }
            break;
        }
    }
}



#endif			//!PROCESS_UTF_INCLUDE

