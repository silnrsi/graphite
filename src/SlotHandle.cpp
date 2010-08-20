#include "graphiteng/SlotHandle.h"
#include "Slot.h"

using namespace org::sil::graphite::v2;

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


float SlotHandle::advance(const GrFont *font) const
{
    return font->advance(m_p->m_glyphid);
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
    return m_p->getAttr(hSeg.ptr(), index, subindex);
}

bool SlotHandle::isInsertBefore() const
{
    return m_p->isInsertBefore();
}

int SlotHandle::original() const
{
    return m_p->original();
}

SlotHandle SlotHandle::next() const
{
    return m_p->next();
}

bool SlotHandle::isNull() const
{
    return (!m_p);
}

size_t SlotHandle::id() const
{
    return size_t(m_p);
}

size_t SlotHandle::attachedTo() const
{
    return size_t(m_p->attachedTo());
}

