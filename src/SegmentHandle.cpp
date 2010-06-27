#include "graphiteng/SegmentHandle.h"
#include "Segment.h"
#include "graphiteng/ITextSource.h"

GRNG_EXPORT void DeleteSegment(Segment *p)
{
    delete p;
}


SegmentHandle::SegmentHandle(const LoadedFont *font, const LoadedFace *face, const ITextSource *txt)
{
    initialize(font, face, face->theFeatures().cloneFeatures(0/*0 means default*/), txt);
}


SegmentHandle::SegmentHandle(const LoadedFont *font, const LoadedFace *face, const FeaturesHandle& pFeats/*must not be IsNull*/, const ITextSource *txt)
{
    initialize(font, face, pFeats, txt);
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


SlotHandle SegmentHandle::operator[] (unsigned int index) const
{
    return &(ptr()->operator[](index));
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


void SegmentHandle::initialize(const LoadedFont *font, const LoadedFace *face, const FeaturesHandle& pFeats/*must not be IsNull*/, const ITextSource *txt)
{
    int numchars = txt->getLength();
    setPtr(new Segment(numchars, face));

    ptr()->chooseSilf(0);
    ptr()->read_text(face, pFeats, txt, numchars);
    ptr()->runGraphite();
    // run the line break passes
    // run the substitution passes
    ptr()->prepare_pos(font);
    // run the positioning passes
    ptr()->finalise(font);
#ifndef DISABLE_TRACING
    ptr()->logSegment(*txt);
#endif
}


