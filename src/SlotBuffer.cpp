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
#if defined(GRAPHITE2_NTRACING)
    constexpr size_t const DEBUG_ATTRS = 0;
#else
    constexpr size_t const DEBUG_ATTRS = 1;
#endif
}

SlotBuffer::SlotBuffer(size_t chunk_size, size_t num_user)
: _free_list{nullptr},
  _head{&_head,&_head},
  _size{0},
  _attrs_size{num_user},
  _chunk_size{chunk_size}
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
    for (auto&& buf: _slots_storage) free(buf);
    for (auto&& buf: _attrs_storage) free(buf);
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

void SlotBuffer::push_back(value_type const & slot)
{
    insert(end(), slot);
}

void SlotBuffer::push_back(value_type && slot)
{
    insert(end(), slot);
}

auto SlotBuffer::erase(iterator pos) -> iterator
{
    assert(pos._p);
    auto node = iterator::node(pos++);
    node->unlink();
    _free_node(node);
    --_size;
    return pos;
}

auto SlotBuffer::erase(iterator first, iterator const last) -> iterator
{
    assert(last._p);
    while (first != last) erase(first++);
    return first;
}

SlotBuffer::_node<Slot> * SlotBuffer::_allocate_node()
{
    if (!_free_list)
    {
        // This is the actual per-slot allocated space, which can be different
        // from the user attributes availble to the action & constraint code.
        auto const real_attr_size = _attrs_size + DEBUG_ATTRS;
        auto nodes = grzeroalloc<_node<value_type>>(_chunk_size);
        auto attrs = grzeroalloc<int16>(_chunk_size * real_attr_size);
        if (!nodes || !attrs)
        {
            free(nodes);
            free(attrs);
            return nullptr;
        }

        for (size_t i = 0; i != _chunk_size; ++i)
        {
            ::new (&nodes[i]._value) Slot(attrs + i*real_attr_size);
            nodes[i]._next = nodes + i + 1;
        }
        nodes[_chunk_size - 1]._next = nullptr;
        _slots_storage.push_back(nodes);
        _attrs_storage.push_back(attrs);
        _free_list = nodes;
    }
    auto node = _free_list;
    _free_list = _free_list->_next;
    node->_next = node->_prev = node;
    return static_cast<_node<value_type> *>(node);
}

void SlotBuffer::_free_node(_node<value_type> * const node)
{
    if (!node) return;

    // Destroy the slot
    node->_value.Slot::~Slot();
    // reset the slot in case it is reused
    ::new (&node->_value) Slot(node->_value.userAttrs());
    // Do NOT clear attr above DEBUG_ATTRS above _attr_size.
    memset(node->_value.userAttrs(), 0, _attrs_size * sizeof(int16));
#if !defined GRAPHITE2_NTRACING
    ++node->_value.userAttrs()[_attrs_size];
#endif
    // update next pointer
    node->_next = _free_list;
    node->_prev = nullptr;
    _free_list = node;
}

namespace {
    constexpr int8_t BIDI_MARK = 0x10;
}

// reverse the clusters: keep diacritics in their original order w.r.t their base character.
void SlotBuffer::reverse()
{
    auto s = begin();
    while (s != end() && s->getBidiClass() == BIDI_MARK) ++s;

    while (s != end())
    {
        auto const c = s++;
        while (s != end() && s->getBidiClass() == BIDI_MARK) ++s;
        _head._next->splice(*iterator::node(c),*iterator::node(std::prev(s)));
    }
}

