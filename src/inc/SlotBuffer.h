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

#include <iterator>

#include "inc/List.h"
#include "inc/Main.h"
#include "inc/Slot.h"

#define MAX_SEG_GROWTH_FACTOR  64

namespace graphite2 {


class SlotBuffer
{
    template <class T>
    class _iterator
    {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = T;
        using difference_type = ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type&;

    private:
        pointer _p;

    public:
        _iterator(pointer p=nullptr): _p(p) {}
        
        bool operator==(_iterator<T> rhs) const { return _p == rhs._p; }
        bool operator!=(_iterator<T> rhs) const { return !operator==(rhs); }

        reference operator*() const { return *_p; }
        pointer operator->() const { return _p; }

        _iterator<T> &  operator++() { _p = _p->next(); return *this; }
        _iterator<T>    operator ++ (int)   { _iterator<T> tmp(*this); operator++(); return tmp; }

         _iterator<T> &   operator--() { _p = _p->prev(); return *this; }
        _iterator<T>    operator--(int)   { _iterator<T> tmp(*this); operator--(); return tmp; }

        pointer ptr() const { return _p; }
        operator pointer() const { return _p; }
    };

    using SlotStore = Vector<Slot *>;
    using AttributeStore = Vector<int16 *>;

    // // Prevent copying of any kind.
    // SlotBuffer& operator=(const SlotBuffer&);

public:
    using value_type = Slot;
    using reference = value_type&;
    using const_reference = const value_type&;
    using iterator = _iterator<value_type>;
    using const_iterator = _iterator<const value_type>;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using difference_type = ptrdiff_t;
    using size_type = size_t;

    SlotBuffer(size_t chunk_size = 16, size_t num_user = 32);
//     template<class I>
//     SlotBuffer(size_t num_user, I &start, const I &end);
    ~SlotBuffer();


//     size_t size() const { return m_size; }

    iterator first() { return begin(); }
    const_iterator first() const { return begin(); }
    void first(Slot *p) { m_first = p; }
    iterator last() { return iterator(&back()); }
    const_iterator last() const { return const_iterator(&back()); }
    void last(Slot *p) { m_last = p; }
//     void push_back(const_reference);
    Slot *newSlot();
    void freeSlot(pointer);
    bool grow() const { return !m_free; }

//     void reverse();

    CLASS_NEW_DELETE


    iterator        begin()         { return iterator(m_first); }
    const_iterator  begin() const   { return cbegin(); }
    iterator        end()           { return iterator(); }
    const_iterator  end() const     { return cend(); }
    const_iterator  cbegin() const  { return const_iterator(m_first); }
    const_iterator  cend() const    { return const_iterator(); }

    reference       front()         { return *m_first; }
    const_reference front() const   { return *m_first; }
    reference       back()          { return *m_last; }
    const_reference back() const    { return *m_last; }

private:
    // pointer link(pointer pos, reference val);
    pointer unlink(reference pos);

    SlotStore       m_slots;        // Vector of slot buffers
    AttributeStore  m_attrs;        // Vector of userAttrs buffers
    pointer         m_first,        // first slot in segment
                    m_last,         // last slot in segment
                    m_free;         // linked list of free slots
    size_t          m_size;
    size_t const    m_attrs_size,   // Per-slot user attributes
                    m_chunk_size;

};

} // namespace graphite2
