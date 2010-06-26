#include "graphiteng/SegmentHandle.h"
#include "Segment.h"
#include "graphiteng/ITextSource.h"

GRNG_EXPORT void DeleteSegment(Segment *p)
{
    delete p;
}


SegmentHandle::SegmentHandle(const LoadedFont *font, const LoadedFace *face, const ITextSource *txt)
{
    int numchars = txt->getLength();
    SetPtr(new Segment(numchars, face));

    Ptr()->chooseSilf(0);
    Ptr()->read_text(face, txt, numchars);
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


int SegmentHandle::length() const
{
    return Ptr()->length();
}


Position SegmentHandle::advance() const
{
    return Ptr()->advance();
}


SlotHandle SegmentHandle::operator[] (int index) const
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


