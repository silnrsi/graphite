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


class IgnoreErrors
{
public:
    static bool handleError(const void* pPositionOfError) { return true;}
};


class BreakOnError
{
public:
    BreakOnError() : m_pErrorPos(NULL) {}
    
    bool handleError(const void* pPositionOfError) { m_pErrorPos=pPositionOfError; return false;}

public:
    const void* m_pErrorPos;
};





/*
  const int utf8_extrabytes_lut[16] = {0,0,0,0,0,0,0,0,        // 1 byte
                                          3,3,3,3,  // errors since trailing byte, catch later
                                          1,1,            // 2 bytes
                                          2,                 // 3 bytes
                                          3};                // 4 bytes
   quicker to implement directly:
*/

inline unsigned int utf8_extrabytes(const unsigned int topNibble) { return (0xE5FF0000>>(2*topNibble))&0x3; }

inline unsigned int utf8_mask(const unsigned int seq_extra) { return ((0xFEC0>>(4*seq_extra))&0xF)<<4; }

class Utf8Consumer
{
public:
    Utf8Consumer(const uint8* pCharStart2) : m_pCharStart(pCharStart2) {}
    
    const uint8* pCharStart() const { return m_pCharStart; }

    template <class LIMIT, class ERRORHANDLER>
    inline bool consumeChar(const LIMIT& limit, uint32* pRes, ERRORHANDLER* pErrHandler) {			//At start, limit.inBuffer(m_pCharStart) is true. return value is iff character contents does not go past limit
        const unsigned int seq_extra = utf8_extrabytes(*m_pCharStart >> 4);        //length of sequence including *m_pCharStart is 1+seq_extra
        if (!limit.inBuffer(m_pCharStart+(seq_extra))) {
            return false;
        }
    
        *pRes = *m_pCharStart ^ utf8_mask(seq_extra);
        
        if (seq_extra) {
            switch(seq_extra) {    //hopefully the optimizer will implement this as a jump table. If not the above if should cover the majority case.    
                case 3: {	
                    if ((*m_pCharStart>>3)==0x1E) {		//the good case
                        ++m_pCharStart;
                        if ((*m_pCharStart&0xC0)!=0x80) {
                            *pRes = 0xFFFD;
                            if (!pErrHandler->handleError(m_pCharStart)) {
                                return false;
                            }                          
                            ++m_pCharStart; 
                            return true;
                        }           
                          
                        *pRes <<= 6; *pRes |= *m_pCharStart & 0x3F;		//drop through
                    }
                    else {
                        *pRes = 0xFFFD;
                        if (!pErrHandler->handleError(m_pCharStart)) {
                          return false;
                        }                          
                        ++m_pCharStart; 
                        return true;
                    }		    
                }
                case 2: {
                        ++m_pCharStart;
                        if ((*m_pCharStart&0xC0)!=0x80) {
                            *pRes = 0xFFFD;
                            if (!pErrHandler->handleError(m_pCharStart)) {
                                return false;
                            }                          
                            ++m_pCharStart; 
                            return true;
                        }
                }           
                *pRes <<= 6; *pRes |= *m_pCharStart & 0x3F;       //drop through
                case 1: {
                        ++m_pCharStart;
                        if ((*m_pCharStart&0xC0)!=0x80) {
                            *pRes = 0xFFFD;
                            if (!pErrHandler->handleError(m_pCharStart)) {
                                return false;
                            }                          
                            ++m_pCharStart; 
                            return true;
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
private:
    static const unsigned int SURROGATE_OFFSET = 0x10000 - 0xDC00;

public:
      Utf16Consumer(const uint16* pCharStart2) : m_pCharStart(pCharStart2) {}
      
      const uint16* pCharStart() const { return m_pCharStart; }
  
      template <class LIMIT, class ERRORHANDLER>
      inline bool consumeChar(const LIMIT& limit, uint32* pRes, ERRORHANDLER* pErrHandler)			//At start, limit.inBuffer(m_pCharStart) is true. return value is iff character contents does not go past limit
      {
	  *pRes = *m_pCharStart;
      if (0xD800 > *pRes || *pRes >= 0xE000) {
          ++m_pCharStart;
          return true;
      }
      
      if (*pRes >= 0xDC00) {        //second surrogate is incorrectly coming first
          *pRes = 0xFFFD;
          if (!pErrHandler->handleError(m_pCharStart)) {
            return false;
          }
          ++m_pCharStart;
          return true;
      }

      ++m_pCharStart;
	  if (!limit.inBuffer(m_pCharStart)) {
	      return false;
	  }

	  uint32 ul = *(m_pCharStart);
	  if (0xDC00 > ul || ul > 0xDFFF) {
	      *pRes = 0xFFFD;
          if (!pErrHandler->handleError(m_pCharStart)) {
            return false;
          }
          ++m_pCharStart;
	      return true;
	  }
	  ++m_pCharStart;
	  *pRes =  (*pRes<<10) + ul + SURROGATE_OFFSET;
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
  
      template <class LIMIT, class ERRORHANDLER>
      inline bool consumeChar(const LIMIT& limit, uint32* pRes, ERRORHANDLER* pErrHandler)			//At start, limit.inBuffer(m_pCharStart) is true. return value is iff character contents does not go past limit
      {
	  *pRes = *m_pCharStart;
      if (*pRes<0xD800) {
          ++m_pCharStart;
          return true;
      }
      
      if (*pRes>=0xE000 && *pRes<0x110000) {
          ++m_pCharStart;
          return true;
      }
        
      *pRes = 0xFFFD;
      if (!pErrHandler->handleError(m_pCharStart)) {
        return false;
      }
      ++m_pCharStart;
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

class ERRORHANDLER
{
public:
    bool handleError(const void* pPositionOfError);     //returns true iff error handled and should continue
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

Useful examples of ERRORHANDLER are IgnoreErrors, BreakOnError.
*/

template <class LIMIT, class CHARPROCESSOR, class ERRORHANDLER>
void processUTF(const LIMIT& limit/*when to stop processing*/, CHARPROCESSOR* pProcessor, ERRORHANDLER* pErrHandler)
{
     uint32             cid;
     switch (limit.enc()) {
       case SegmentHandle::kutf8 : {
	    Utf8Consumer consumer(static_cast<const uint8 *>(limit.pStart()));
            for (;limit.needMoreChars(consumer.pCharStart(), pProcessor->charsProcessed());) {
		if (!consumer.consumeChar(limit, &cid, pErrHandler))
		    break;
		if (!pProcessor->processChar(cid))
		    break;
            }
            break;
        }
       case SegmentHandle::kutf16: {
            Utf16Consumer consumer(static_cast<const uint16 *>(limit.pStart()));
            for (;limit.needMoreChars(consumer.pCharStart(), pProcessor->charsProcessed());) {
		if (!consumer.consumeChar(limit, &cid, pErrHandler))
		    break;
		if (!pProcessor->processChar(cid))
		    break;
            }
	    break;
        }
       case SegmentHandle::kutf32 : default: {
	    Utf32Consumer consumer(static_cast<const uint32 *>(limit.pStart()));
            for (;limit.needMoreChars(consumer.pCharStart(), pProcessor->charsProcessed());) {
		if (!consumer.consumeChar(limit, &cid, pErrHandler))
		    break;
		if (!pProcessor->processChar(cid))
		    break;
            }
            break;
        }
    }
}



#endif			//!PROCESS_UTF_INCLUDE

