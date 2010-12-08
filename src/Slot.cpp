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
#include "graphite2/Segment.h"
#include "SlotImp.h"

using namespace org::sil::graphite::v2;

extern "C" 
{
GRNG_EXPORT const GrSlot* gr_slot_next_in_segment(const GrSlot* p/*not NULL*/)
{
    assert(p);
    return p->next();
}

GRNG_EXPORT const GrSlot* gr_slot_prev_in_segment(const GrSlot* p/*not NULL*/)
{
    assert(p);
    return p->prev();
}

GRNG_EXPORT const GrSlot* gr_slot_attached_to(const GrSlot* p/*not NULL*/)        //returns NULL iff base. If called repeatedly on result, will get to a base
{
    assert(p);
    return p->attachTo();
}


GRNG_EXPORT const GrSlot* gr_slot_first_attachment(const GrSlot* p/*not NULL*/)        //returns NULL iff no attachments.
{        //if slot_first_attachment(p) is not NULL, then slot_attached_to(slot_first_attachment(p))==p.
    assert(p);
    return p->firstChild();
}

    
GRNG_EXPORT const GrSlot* gr_slot_next_sibling_attachment(const GrSlot* p/*not NULL*/)        //returns NULL iff no more attachments.
{        //if slot_next_sibling_attachment(p) is not NULL, then slot_attached_to(slot_next_sibling_attachment(p))==slot_attached_to(p).
    assert(p);
    return p->nextSibling();
}


GRNG_EXPORT unsigned short gr_slot_gid(const GrSlot* p/*not NULL*/)
{
    assert(p);
    return p->gid();
}


GRNG_EXPORT float gr_slot_origin_X(const GrSlot* p/*not NULL*/)
{
    assert(p);
    return p->origin().x;
}


GRNG_EXPORT float gr_slot_origin_Y(const GrSlot* p/*not NULL*/)
{
    assert(p);
    return p->origin().y;
}


GRNG_EXPORT float gr_slot_advance(const GrSlot* p/*not NULL*/)
{
    assert(p);
    return p->advance();
}

GRNG_EXPORT int gr_slot_before(const GrSlot* p/*not NULL*/)
{
    return p->before();
}


GRNG_EXPORT int gr_slot_after(const GrSlot* p/*not NULL*/)
{
    return p->after();
}

GRNG_EXPORT int gr_slot_attr(const GrSlot* p/*not NULL*/, const GrSegment* pSeg/*not NULL*/, attrCode index, uint8 subindex)
{
    return p->getAttr(pSeg, index, subindex);
}


GRNG_EXPORT bool gr_slot_can_insert_before(const GrSlot* p/*not NULL*/)
{
    return p->isInsertBefore();
}


GRNG_EXPORT int gr_slot_original(const GrSlot* p/*not NULL*/)
{
    return p->original();
}


#if 0       //what should this be
GRNG_EXPORT size_t id(const Slot* p/*not NULL*/)
{
    return (size_t)p->id();
}
#endif


}


