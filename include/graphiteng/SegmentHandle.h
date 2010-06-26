#ifndef SEGMENT_HANDLE_INCLUDE
#define SEGMENT_HANDLE_INCLUDE

#include "graphiteng/Types.h"
#include "graphiteng/RefCountHandle.h"

class ISlot;
class Segment;
class ITextSource;
class LoadedFace;
class LoadedFont;

extern GRNG_EXPORT void DeleteSegment(Segment *p);

class GRNG_EXPORT SegmentHandle : public RefCountHandle<Segment, &DeleteSegment>
{
public:
    SegmentHandle(const LoadedFont *font, const LoadedFace *face, const ITextSource *txt);
  
    int length() const;
    Position advance() const;
    ISlot & operator[] (int index) const;
    void runGraphite();
    void chooseSilf(uint32 script);
};


#endif // !SEGMENT_HANDLE_INCLUDE
