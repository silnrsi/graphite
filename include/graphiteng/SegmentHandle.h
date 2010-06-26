#ifndef SEGMENT_HANDLE_INCLUDE
#define SEGMENT_HANDLE_INCLUDE

#include "graphiteng/Types.h"
#include "graphiteng/RefCountHandle.h"
#include "graphiteng/SlotHandle.h"

class Segment;
class ITextSource;
class LoadedFace;
class LoadedFont;
class FeaturesHandle;

extern GRNG_EXPORT void DeleteSegment(Segment *p);

class GRNG_EXPORT SegmentHandle : public RefCountHandle<Segment, &DeleteSegment>
{
public:
    SegmentHandle(const LoadedFont *font, const LoadedFace *face, const ITextSource *txt);
    SegmentHandle(const LoadedFont *font, const LoadedFace *face, const FeaturesHandle& pFeats/*must not be IsNull*/, const ITextSource *txt);
  
    int length() const;
    Position advance() const;
    SlotHandle operator[] (int index) const;
    void runGraphite() const;
    void chooseSilf(uint32 script) const;

private:
    void initialize(const LoadedFont *font, const LoadedFace *face, const FeaturesHandle& pFeats/*must not be IsNull*/, const ITextSource *txt);
    
private:
    friend class SlotHandle;
    Segment* Ptr() const { return RefCountHandle<Segment, &DeleteSegment>::Ptr(); }
};


#endif // !SEGMENT_HANDLE_INCLUDE
