#ifndef SEGMENT_HANDLE_INCLUDE
#define SEGMENT_HANDLE_INCLUDE

#include "graphiteng/Types.h"
#include "graphiteng/RefCountHandle.h"
#include "graphiteng/SlotHandle.h"

class Segment;
class LoadedFace;
class LoadedFont;
class FeaturesHandle;

extern GRNG_EXPORT void DeleteSegment(Segment *p);

class GRNG_EXPORT SegmentHandle : public RefCountHandle<Segment, &DeleteSegment>
{
public:
    enum encform {
	  kutf8 = 1/*sizeof(uint8)*/, kutf16 = 2/*sizeof(uint16)*/, kutf32 = 4/*sizeof(uint32)*/
    };

    static size_t countUnicodeCharacters(encform enc, const void* buffer_begin, const void* buffer_end/*as in stl i.e. don't use end*/);
    static size_t countUnicodeCharacters(encform enc, const void* buffer_begin, const void* buffer_end/*as in stl i.e. don't use end*/, size_t maxCount);
    static size_t countUnicodeCharactersToNul(encform enc, const void* buffer_begin);	//the nul is not in the count
    static size_t countUnicodeCharactersToNul(encform enc, const void* buffer_begin, const void* buffer_end/*don't go on or past end*/);	//the nul is not in the count
    static size_t countUnicodeCharactersToNul(encform enc, const void* buffer_begin, const void* buffer_end/*don't go on or past end*/, size_t maxCount);	//the nul is not in the count
    
public:
    SegmentHandle(const LoadedFont *font, const LoadedFace *face, encform enc, const void* pStart, size_t nChars);
    SegmentHandle(const LoadedFont *font, const LoadedFace *face, const FeaturesHandle& pFeats/*must not be isNull*/, encform enc, const void* pStart, size_t nChars);
  
    int length() const;
    float advanceX() const;
    float advanceY() const;
    SlotHandle operator[] (unsigned int index) const;
    void runGraphite() const;
    void chooseSilf(uint32 script) const;
    
    int addFeatures(const FeaturesHandle& feats) const;
 
private:
    void initialize(const LoadedFont *font, const LoadedFace *face, const FeaturesHandle& pFeats/*must not be isNull*/, encform enc, const void*pStart, size_t nChars);
    
private:
    friend class SlotHandle;
    Segment* ptr() const { return RefCountHandle<Segment, &DeleteSegment>::ptr(); }
};



#endif // !SEGMENT_HANDLE_INCLUDE
