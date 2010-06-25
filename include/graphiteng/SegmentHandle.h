#ifndef SEGMENT_HANDLE_INCLUDE
#define SEGMENT_HANDLE_INCLUDE

#include "graphiteng/Types.h"

class ISlot;
class Segment;
class ITextSource;
class LoadedFace;
class LoadedFont;

class SegmentHandle 
{
public:
    SegmentHandle(const LoadedFont *font, const LoadedFace *face, const ITextSource *txt);
    ~SegmentHandle();
  
    int length() const;
    Position advance() const;
    ISlot & operator[] (int index) const;
    void runGraphite();
    void chooseSilf(uint32 script);
    
private:
    Segment* m_pSegment;		//not NULL
    
    SegmentHandle(const SegmentHandle&);
    SegmentHandle&operator=(const SegmentHandle&);
};


#endif // !SEGMENT_HANDLE_INCLUDE
