/*  GRAPHITE2 LICENSING

    Copyright 2019, SIL International
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

#pragma once

#include <cstddef>
// #include "inc/Code.h"
#include "SlotBuffer.h"
#include "inc/Slot.h"

namespace graphite2 {

class SlotMap
{
public:
    using value_type = SlotBuffer::iterator;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using difference_type = ptrdiff_t;
    using size_type = size_t;

    enum {MAX_SLOTS=64};
    SlotMap(Segment & seg, uint8 direction, size_t maxSize);

    iterator        begin();
    iterator        end();
    size_type       size() const;
    size_type       context() const;
    void            reset(Slot &, unsigned short);

    const_reference operator[](int n) const;
    reference       operator [] (int);
    void            pushSlot(value_type slot);
    void            collectGarbage(reference aSlot);

    value_type      highwater() { return m_highwater; }
    void            highwater(value_type s) { m_highwater = s; m_highpassed = false; }
    bool            highpassed() const { return m_highpassed; }
    void            highpassed(bool v) { m_highpassed = v; }

    uint8          dir() const { return m_dir; }
    int            decMax() { return --m_maxSize; }

    Segment &    segment;
private:
    value_type      m_slot_map[MAX_SLOTS+1];
    unsigned short  m_size;
    unsigned short  m_precontext;
    value_type      m_highwater;
    int             m_maxSize;
    uint8           m_dir;
    bool            m_highpassed;
};

inline
SlotMap::SlotMap(Segment & seg, uint8 direction, size_t maxSize)
: segment(seg), m_size(0), m_precontext(0), m_highwater(nullptr),
    m_maxSize(int(maxSize)), m_dir(direction), m_highpassed(false)
{
    m_slot_map[0] = nullptr;
}

inline
SlotMap::iterator SlotMap::begin()
{
  return &m_slot_map[1]; // allow map to go 1 before slot_map when inserting
                         // at start of segment.
}

inline
SlotMap::iterator SlotMap::end()
{
  return m_slot_map + m_size + 1;
}

inline
SlotMap::size_type SlotMap::size() const
{
  return m_size;
}

inline
SlotMap::size_type SlotMap::context() const
{
  return m_precontext;
}

inline
void SlotMap::reset(Slot & slot, short unsigned int ctxt)
{
  m_size = 0;
  m_precontext = ctxt;
  *m_slot_map = slot.prev();
}

inline
void SlotMap::pushSlot(value_type slot)
{
  m_slot_map[++m_size] = slot;
}

inline
SlotMap::const_reference SlotMap::operator[](int n) const
{
  return m_slot_map[n + 1];
}

inline
SlotMap::reference SlotMap::operator[](int n)
{
  return m_slot_map[n + 1];
}

} // namespace graphite2
