/*  GRAPHITE2 LICENSING

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
    If not, write to the Free Software Foundation, 51 Franklin Street,
    Suite 500, Boston, MA 02110-1335, USA or visit their web page on the
    internet at http://www.fsf.org/licenses/lgpl.html.

Alternatively, the contents of this file may be used under the terms of the
Mozilla Public License (http://mozilla.org/MPL) or the GNU General Public
License, as published by the Free Software Foundation, either version 2
of the License or (at your option) any later version.
*/
#include <cstdlib>

#include "inc/bits.h"
#include "inc/SlotBuffer.h"


using namespace graphite2;

namespace 
{
#if defined(GRAPHITE2_NTRACING)
    constexpr size_t const EXTRA_ATTRS = 0;
#else
    constexpr size_t const EXTRA_ATTRS = 1;
#endif
}

SlotBuffer::SlotBuffer(size_t chunk_size, size_t num_user)
: m_first(nullptr),
  m_last(nullptr),
  m_free(nullptr),
  m_size(0),
  m_attrs_size(num_user),
  m_chunk_size(chunk_size)
{
}

// template<class I>
// SlotBuffer:: SlotBuffer(size_t num_user, I &start, const I &end)
// : SlotBuffer(log_binary(m_size)+1, num_user)
// {
//     m_size = end-start;
//     freeSlot(newSlot());

//     for(; start != end; ++start) push_back(*start);
// }

SlotBuffer::~SlotBuffer()
{
    for (auto&& buf: m_slots) free(buf);
    for (auto&& buf: m_attrs) free(buf);
}

// SlotBuffer::pointer SlotBuffer::link(pointer pos, reference val)
// {
//     pointer const prev = pos ? pos->prev() : m_last;

//     val.next(pos);
//     val.prev(prev);
//     if (prev)   prev->next(&val);
//     else        m_first = &val;

//     if (pos)  pos->prev(&val);
//     else      m_last = &val;
    
//     ++m_size;

//     return &val;
// }

// SlotBuffer::pointer SlotBuffer::unlink(reference val)
// {
//     pointer const prev = val.prev();
//     pointer const next = val.next();

//     val.next(nullptr);
//     val.prev(nullptr);

//     if (prev)   prev->next(next);
//     else        m_first = next;

//     if (next)  next->prev(prev);
//     else      m_last = prev;
    
//     --m_size;

//     return next;
// }


// void SlotBuffer::push_back(const_reference slot)
// {
//     Slot *slot_ptr = newSlot();
//     if (!slot_ptr) return;

//     slot_ptr->set(slot, 0, m_attrs_size, 0, slot.after()+1);
//     link(m_last, *slot_ptr);
// }

Slot * SlotBuffer::newSlot()
{
    if (!m_free)
    {
        // check that the segment doesn't grow indefinintely
        int numUser = m_attrs_size + EXTRA_ATTRS;
        Slot *newSlots = grzeroalloc<Slot>(m_chunk_size);
        int16 *newAttrs = grzeroalloc<int16>(m_chunk_size * numUser);
        if (!newSlots || !newAttrs)
        {
            free(newSlots);
            free(newAttrs);
            return NULL;
        }
        for (size_t i = 0; i < m_chunk_size; i++)
        {
            ::new (newSlots + i) Slot(newAttrs + i * numUser);
            newSlots[i].next(newSlots + i + 1);
        }
        newSlots[m_chunk_size - 1].next(NULL);
        m_slots.push_back(newSlots);
        m_attrs.push_back(newAttrs);
        m_free = newSlots;
    }
    Slot *res = m_free;
    m_free = m_free->next();
    res->next(nullptr);
    return res;
}

void SlotBuffer::freeSlot(pointer aSlot)
{
    if (aSlot == nullptr) return;
    if (m_last == aSlot) last(aSlot->prev());
    if (m_first == aSlot) first(aSlot->next());
    // reset the slot incase it is reused
    ::new (aSlot) Slot(aSlot->userAttrs());
    memset(aSlot->userAttrs(), 0, m_attrs_size * sizeof(int16));
#if !defined GRAPHITE2_NTRACING
    ++aSlot->userAttrs()[m_attrs_size];
#endif
    // update next pointer
    aSlot->next(m_free);
    m_free = aSlot;
}


// // reverse the slots but keep diacritics in their same position after their bases
// void SlotBuffer::reverse()
// {
//     if (m_first == m_last) return;      // skip 0 or 1 glyph runs

//     Slot *t = 0;
//     Slot *curr = m_first;
//     Slot *tlast;
//     Slot *tfirst;
//     Slot *out = nullptr;

//     while (curr && curr->getBidiClass() == 16) curr = curr->next();
//     if (!curr) return;
//     tfirst = curr->prev();
//     tlast = curr;

//     while (curr)
//     {
//         if (curr->getBidiClass() == 16)
//         {
//             Slot *d = curr->next();
//             while (d && d->getBidiClass() == 16) d = d->next();

//             d = d ? d->prev() : m_last;
//             Slot *p = out->next();    // one after the diacritics. out can't be null
//             if (p)
//                 p->prev(d);
//             else
//                 tlast = d;
//             t = d->next();
//             d->next(p);
//             curr->prev(out);
//             out->next(curr);
//         }
//         else    // will always fire first time round the loop
//         {
//             if (out)
//                 out->prev(curr);
//             t = curr->next();
//             curr->next(out);
//             out = curr;
//         }
//         curr = t;
//     }
//     out->prev(tfirst);
//     if (tfirst)
//         tfirst->next(out);
//     else
//         m_first = out;
//     m_last = tlast;
// }

