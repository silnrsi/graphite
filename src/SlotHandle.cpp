#include "graphiteng/SlotHandle.h"
#include "Slot.h"
#include "graphiteng/ITextSource.h"

unsigned short SlotHandle::gid() const
{
    return m_p->gid();
}


float SlotHandle::originX() const
{
    return m_p->origin().x;
}


float SlotHandle::originY() const
{
    return m_p->origin().y;
}


float SlotHandle::advance(const LoadedFont *font) const
{
    return m_p->advance(font);
}


int SlotHandle::before() const
{
    return m_p->before();
}


int SlotHandle::after() const
{
    return m_p->after();
}


int SlotHandle::getAttr(const SegmentHandle& hSeg, attrCode index, uint8 subindex) const
{
    return m_p->getAttr(hSeg.Ptr(), index, subindex);
}


