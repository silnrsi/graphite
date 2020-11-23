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
    // @DEBUG_ATTRS: The number of extra per-slot user attributes beyond the
    // number declared in by the font. Used to store persistent debug
    // information that is not zeroed by SlotBuffer::_free_node(). 
// #if defined(GRAPHITE2_NTRACING)
    constexpr size_t const DEBUG_ATTRS = 0;
// #else
//     constexpr size_t const DEBUG_ATTRS = 1;
// #endif
}


SlotBuffer::SlotBuffer(size_t num_user)
: _size{0},
  _attrs_size{num_user}
{
}

SlotBuffer::SlotBuffer(SlotBuffer && rhs)
: _size{rhs._size},
  _attrs_size{rhs._attrs_size}
{
    if (!rhs.empty())
        _head.splice(*rhs._head._next, *rhs._head._prev);

    if (rhs._garbage._next != &rhs._garbage)
        _garbage.splice(*rhs._garbage._next, *rhs._garbage._prev);
}

SlotBuffer & SlotBuffer::operator = (SlotBuffer && rhs) {
    clear();
    ::new (this) SlotBuffer(std::move(rhs));
    return *this;
}

void SlotBuffer::_node_linkage::splice(_node_linkage & start, _node_linkage & end)
{
    // Unlink from current position
    start._prev->_next = end._next;
    end._next->_prev = start._prev;

    // Link in before this node.
    start._prev = this->_prev;
    end._next = this;
    start._prev->_next = &start;
    end._next->_prev = &end;
}

inline 
void SlotBuffer::_node_linkage::link(_node_linkage & pos)
{
    pos.splice(*this, *this);
}

void SlotBuffer::_node_linkage::unlink()
{
    _prev->_next = _next;
    _next->_prev = _prev;
    _prev = _next = this;
}

SlotBuffer::iterator SlotBuffer::insert(const_iterator pos, value_type const & slot)
{
    assert(pos._p);
    auto node = _allocate_node();
    if (!node) return end();

    node->_value.set(slot, 0, _attrs_size, 0, slot.after()+1);
    node->link(*const_cast<_node_linkage *>(pos._p));

    ++_size;
    return iterator(node);
}

SlotBuffer::iterator SlotBuffer::insert(const_iterator pos, value_type && slot)
{
    assert(pos._p);
    auto node = _allocate_node();
    if (!node) return end();

    node->_value.set(slot, 0, _attrs_size, 0, slot.after()+1);
    node->link(*const_cast<_node_linkage *>(pos._p));

    ++_size;
    return iterator(node);
}

void SlotBuffer::push_back(value_type const & v)
{
    insert(end(), v);
}

void SlotBuffer::push_back(value_type && v)
{
    insert(end(), std::forward<value_type>(v));
}

void SlotBuffer::splice(const_iterator pos, SlotBuffer &other, const_iterator first, const_iterator last)
{
    if (first != last)
    {
        auto l = std::distance(first, last);
        const_cast<_node_linkage *>(pos._p)->splice(const_cast<_node_linkage &>(*first._p), 
                                                    const_cast<_node_linkage &>(*(last._p->_prev)));
        other._size -= l;
        _size += l;
    }
}

auto SlotBuffer::erase(iterator pos) -> iterator
{
    assert(pos._p);
    auto node = iterator::node(pos++);
    _garbage.splice(*node, *node);
    --_size;
    return pos;
}

auto SlotBuffer::erase(iterator first, iterator const last) -> iterator
{
    assert(last._p);
    if (first != last)
    {
        _size -= std::distance(first, last);
        _garbage.splice(const_cast<_node_linkage &>(*first._p), 
                        const_cast<_node_linkage &>(*last._p->_prev));
    }
    return last;
}

void SlotBuffer::collect_garbage(bool only_marked)
{
    auto node = _garbage._next;
    while (node != &_garbage)
    {
        auto n = iterator::node(node);
        node = node->_next;
        _free_node(n);
    }
    _garbage._next = _garbage._prev = &_garbage;
}

auto SlotBuffer::_allocate_node() -> SlotBuffer::_node<value_type> * 
{
    auto const real_attr_size = _attrs_size + DEBUG_ATTRS;
    auto attrs = grzeroalloc<int16>(real_attr_size);
    auto node = attrs ? new _node<value_type>() : nullptr;
    if (!node) { 
        free(attrs); 
        return nullptr;
    }
    node->_value.userAttrs(attrs);
    return static_cast<_node<value_type> *>(node);
}

void SlotBuffer::_free_node(_node<value_type> * const node)
{
    free(node);
}

namespace {
    constexpr int8_t BIDI_MARK = 0x10;

    template <class It>
    inline It skip_bidi_mark(It first, It const last) {
        while (first != last && first->getBidiClass() == BIDI_MARK) ++first;
        return first;
    }

}

// reverse the clusters: keep diacritics in their original order w.r.t their base character.
void SlotBuffer::reverse()
{
    _node_linkage out;

    auto s = skip_bidi_mark(begin(), end());
    if (s == end()) return;

    while (s != end())
    {
        auto const c = const_cast<_node_linkage *>(s._p);
        s = skip_bidi_mark(++s, end());
        out._next->splice(*c, *(s._p->_prev));
    }
    _head.splice(*out._next, *out._prev);
}

