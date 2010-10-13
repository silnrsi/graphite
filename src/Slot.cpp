/*  GRAPHITENG LICENSING

    Copyright 2010, SIL International
    All rights reserved.

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published
    by the Free Software Foundation; either version 2.1 of License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should also have received a copy of the GNU Lesser General Public
    License along with this library in the file named "LICENSE".
    If not, write to the Free Software Foundation, Inc., 59 Temple Place, 
    Suite 330, Boston, MA 02111-1307, USA or visit their web page on the 
    internet at http://www.fsf.org/licenses/lgpl.html.
*/
#include "graphiteng/Slot.h"
#include "SlotImp.h"

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


int SlotHandle::getAttr(const GrSegment* pSeg/*not NULL*/, attrCode index, uint8 subindex) const
{
    return m_p->getAttr(pSeg, index, subindex);
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

