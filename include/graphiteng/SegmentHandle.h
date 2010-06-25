#ifndef SEGMENT_HANDLE_INCLUDE
#define SEGMENT_HANDLE_INCLUDE

#include "graphiteng/Types.h"
#include "graphiteng/RefCountHandle.h"

class ISlot;
class Segment;
class ITextSource;
class LoadedFace;
class LoadedFont;

class SegmentHandle 
{
public:
    SegmentHandle(const LoadedFont *font, const LoadedFace *face, const ITextSource *txt);
    SegmentHandle& operator=(const SegmentHandle& src);
    ~SegmentHandle();
  
    int length() const;
    Position advance() const;
    ISlot & operator[] (int index) const;
    void runGraphite();
    void chooseSilf(uint32 script);
    
    Segment* operator->() const { return m_pSegment; }		//cannot be used by client code - only available witin graphite code!
    
private:
    RefCountHandle m_RefCountHandle;
    Segment* m_pSegment;		//not NULL
    
};


#endif // !SEGMENT_HANDLE_INCLUDE
