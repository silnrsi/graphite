#include "graphiteng/SegmentHandle.h"
#include "Segment.h"
#include "graphiteng/ITextSource.h"

SegmentHandle::SegmentHandle(const LoadedFont *font, const LoadedFace *face, const ITextSource *txt)
{
    int numchars = txt->getLength();
    m_pSegment = new Segment(numchars, face);

    m_pSegment->chooseSilf(0);
    m_pSegment->read_text(face, txt, numchars);
    m_pSegment->runGraphite();
    // run the line break passes
    // run the substitution passes
    m_pSegment->prepare_pos(font);
    // run the positioning passes
    m_pSegment->finalise(font);
#ifndef DISABLE_TRACING
    m_pSegment->logSegment(*txt);
#endif
}


SegmentHandle& SegmentHandle::operator=(const SegmentHandle& src)
{ 
    if ((m_RefCountHandle=src.m_RefCountHandle)==true) 		//I do really mean assignment here - it is overloaded in a nasty way!
	delete m_pSegment; m_pSegment=src.m_pSegment; 
    return *this; 
}


SegmentHandle::~SegmentHandle()
{
    delete m_pSegment;
}


int SegmentHandle::length() const
{
    return m_pSegment->length();
}


Position SegmentHandle::advance() const
{
    return m_pSegment->advance();
}


ISlot & SegmentHandle::operator[] (int index) const
{
    return m_pSegment->operator[](index);
}


void SegmentHandle::runGraphite()
{
    return m_pSegment->runGraphite();
}


void SegmentHandle::chooseSilf(uint32 script)
{
    return m_pSegment->chooseSilf(script);
}


