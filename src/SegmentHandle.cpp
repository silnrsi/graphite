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
    return Ptr()->length();
}


Position SegmentHandle::advance() const
{
    return Ptr()->advance();
}


SlotHandle SegmentHandle::operator[] (unsigned int index) const
{
    return &(Ptr()->operator[](index));
}


void SegmentHandle::runGraphite() const
{
    return Ptr()->runGraphite();
}


void SegmentHandle::chooseSilf(uint32 script) const
{
    return Ptr()->chooseSilf(script);
}


int SegmentHandle::addFeatures(const FeaturesHandle& feats) const
{
    if (feats.IsNull())
	return -2;		//the smallest value that can normally be returned is -1
    
    return Ptr()->addFeatures(*feats.Ptr());
}


void SegmentHandle::initialize(const LoadedFont *font, const LoadedFace *face, const FeaturesHandle& pFeats/*must not be IsNull*/, const ITextSource *txt)
{
    int numchars = txt->getLength();
    SetPtr(new Segment(numchars, face));

    Ptr()->chooseSilf(0);
    Ptr()->read_text(face, pFeats, txt, numchars);
    Ptr()->runGraphite();
    // run the line break passes
    // run the substitution passes
    Ptr()->prepare_pos(font);
    // run the positioning passes
    Ptr()->finalise(font);
#ifndef DISABLE_TRACING
    Ptr()->logSegment(*txt);
#endif
}


