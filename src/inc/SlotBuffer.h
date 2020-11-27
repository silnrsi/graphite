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

#include <array>
#include <iterator>
#include <type_traits>

#include "inc/vector.hpp"
#include "inc/Main.h"
#include "inc/Slot.h"


namespace graphite2 {

class SlotBuffer
{
public:
    using value_type = Slot;
    using difference_type = ptrdiff_t;
    using size_type = size_t;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_nodeointer = const value_type*;

private:
    struct _node_linkage {
        _node_linkage * _next;
        _node_linkage * _prev;

        _node_linkage(): _next{this}, _prev{this} {}
        void link(_node_linkage &);
        void unlink();
        void splice(_node_linkage & start, _node_linkage & end);
    };

    template<typename T>
    struct _node : public _node_linkage {
        T _value;

        CLASS_NEW_DELETE;
        
        template<typename... Args> 
        _node(Args &&... args): _value(std::forward<Args>(args)...) {}
    };

    struct _head_node : public _node_linkage {
        uintptr_t const _head_node_signature;

        uintptr_t get_signature() const {
            return uintptr_t(this);
        }

        _head_node(): _head_node_signature{get_signature()} {}

        bool is_head_node() const {
            return get_signature() == _head_node_signature;
        }
        
        SlotBuffer const * get_container() const {
            return reinterpret_cast<SlotBuffer const *>(_head_node_signature - reinterpret_cast<uintptr_t>(&((SlotBuffer *)0)->_head));
        }

        SlotBuffer * get_container() {
            return const_cast<SlotBuffer *>(const_cast<_head_node const *>(this)->get_container());
        }
    };

    _head_node          _head;
    _node_linkage       _garbage;   // TODO: Move into slot map or merge slotmap into here.
    size_type           _size;
    // uint8_t             _attrs_size,
    //                     _justs_size;

    template<typename T> class _iterator;

    _node<Slot> *   _allocate_node(value_type &&);
    void            _free_node(_node<Slot> * const);
    void            _reverse_range(_node_linkage * const start, _node_linkage * const end);

    SlotBuffer(SlotBuffer && rhs);
    SlotBuffer & operator=(SlotBuffer && rhs);

    CLASS_NEW_DELETE;

public:
    using iterator = _iterator<value_type>;
    using const_iterator = _iterator<const value_type>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;


    // TODO: Retire
    iterator newSlot();
    void     freeSlot(iterator i);

    SlotBuffer(/*size_t num_user, size_t num_justs*/);
    ~SlotBuffer()   { clear(); }

    // size_type       num_user_attrs() const noexcept { return _attrs_size; }
    // size_type       num_just_levels() const noexcept { return _justs_size; }

    iterator        begin() noexcept;
    const_iterator  begin() const noexcept;
    const_iterator  cbegin() const noexcept;

    iterator        end() noexcept;
    const_iterator  end() const noexcept;
    const_iterator  cend() const noexcept;

    // iterator        rbegin()         { return reverse_iterator(_head._next); }
    // const_iterator  rbegin() const   { return cbegin(); }
    // const_iterator  crbegin() const  { return const_reverse_iterator(_head._next); }
    // iterator        rend()           { return reverse_iterator(&_head); }
    // const_iterator  rend() const     { return cend(); }
    // const_iterator  crend() const    { return const_reverse_iterator(&_head); }

    reference       front()         { assert(!empty()); return static_cast<_node<Slot> *>(_head._next)->_value; }
    const_reference front() const   { assert(!empty()); return static_cast<_node<const Slot> *>(_head._next)->_value; }
    reference       back()          { assert(!empty()); return static_cast<_node<Slot> *>(_head._prev)->_value; }
    const_reference back() const    { assert(!empty()); return static_cast<_node<const Slot> *>(_head._prev)->_value; }

    bool empty() const noexcept { return _head._next == &_head || _head._next == nullptr; }
    size_t size() const noexcept { return _size; }

    iterator insert(const_iterator pos, value_type const &);
    iterator insert(const_iterator pos, value_type &&);
    template <typename... Args>
    iterator emplace(const_iterator pos, Args &&... args);

    void push_back(value_type const &v);
    void push_back(value_type &&v);

    template <typename... Args>
    reference emplace_back(Args &&... args) { emplace(end(), std::forward<Args>(args)...); return back(); }

    void pop_back();

    iterator erase(iterator first);
    iterator erase(iterator first, iterator last);

    void clear();

    void reverse();

    void collect_garbage(bool only_marked=false);

    void swap(SlotBuffer & rhs);

    void splice(const_iterator pos, SlotBuffer & other, const_iterator first, const_iterator last);
    void splice(const_iterator pos, SlotBuffer & other);  
    void splice(const_iterator pos, SlotBuffer & other, const_iterator it);
};

template <typename T>
class SlotBuffer::_iterator
{
    _node_linkage const * _p;

    friend SlotBuffer;

    // Find the address of the enclosing object of class S from a pointer to
    // it's contained member m.
    // This is extermely dangerous if performed on a T* that is not an
    // agreggate member of class S.
    template<class S>
    inline
    static constexpr S* container_of(void const *p, T S::* m) noexcept {
        return reinterpret_cast<S *>(
                    reinterpret_cast<uint8_t const *>(p) 
                  - reinterpret_cast<uint8_t const *>(&(((S *)0)->*m)));
    }
    
    static _node<T> * node(_iterator<T> const &i) noexcept { 
        return const_cast<_node<T> *>(static_cast<_node<T> const *>(i._p)); 
    }

    _iterator(std::nullptr_t)  noexcept: _p{nullptr} {}
    _iterator(_node_linkage const *p)  noexcept: _p{p} {}
    friend Segment;

public:
    using iterator_category = std::bidirectional_iterator_tag;
    using difference_type = ptrdiff_t;
    using value_type = T;
    using pointer = value_type*;
    using reference = value_type&;
    using opaque_type = gr_slot const *;

    // TODO: Remove soon
    void next(_iterator<T> i) { assert(_p); const_cast<_node_linkage*>(_p)->_next = const_cast<_node_linkage*>(i._p); }

    void prev(_iterator<T> i) { assert(_p); const_cast<_node_linkage*>(_p)->_prev = const_cast<_node_linkage*>(i._p); }

    static _iterator<T> from(T *r) noexcept {  
        return r ? _iterator<T>{container_of<_node<T>>(r, &_node<T>::_value)} : _iterator<T>{nullptr}; 
    }

    void to_cluster_root() noexcept {
        auto p =  operator->();
        while(!p->isBase())  p = p->attachedTo();
        _p = container_of<_node<T>>(p, &_node<T>::_value);
    }

    _iterator(): _iterator{nullptr} {}
    _iterator(opaque_type p): _iterator{reinterpret_cast<_node_linkage *>(const_cast<gr_slot*>(p))} {}


    bool operator==(_iterator<T> rhs) const noexcept { return _p == rhs._p; }
    bool operator!=(_iterator<T> rhs) const noexcept { return !operator==(rhs); }

    reference operator*() const noexcept { return node(*this)->_value; }
    pointer operator->() const  noexcept { return &operator*(); }

    _iterator<T> &  operator++()     noexcept { assert(_p); _p = _p->_next; return *this; }
    _iterator<T>    operator++(int)  noexcept { _iterator<T> tmp(*this); operator++(); return tmp; }

    _iterator<T> &  operator--() noexcept    { assert(_p); _p = _p->_prev; return *this; }
    _iterator<T>    operator--(int) noexcept { _iterator<T> tmp(*this); operator--(); return tmp; }

    bool        is_valid() const noexcept { return _p != nullptr; }

    opaque_type handle() const noexcept { 
        return _p && !static_cast<_head_node const *>(_p)->is_head_node() 
                    ? reinterpret_cast<opaque_type>(_p) 
                    : nullptr; 
    }
    
    // operator bool() const         noexcept { return _p != nullptr; }
    // operator pointer() const      noexcept { return operator->(); }
    operator _iterator<T const>() const noexcept { return _iterator<T const>(_p); }
};

template <typename... Args>
inline
auto SlotBuffer::emplace(const_iterator pos, Args &&... args) -> iterator {
    assert(pos._p);
    auto node = new _node<value_type>(std::forward<Args>(args)...);
    if (!node) return end();
    node->link(*const_cast<_node_linkage *>(pos._p));
    ++_size;
    return iterator(node);
}

inline
void SlotBuffer::swap(SlotBuffer & rhs) {
    SlotBuffer tmp{std::move(rhs)};
    rhs = std::move(*this);
    *this = std::move(tmp);
}

inline 
auto SlotBuffer::newSlot() -> iterator { 
    auto r = _allocate_node(Slot()); 
    if (!r) return end();
    r->_next = r->_prev = nullptr; 
    return iterator(r); 
}

inline void SlotBuffer::freeSlot(iterator i) { _free_node(iterator::node(i)); }

inline auto SlotBuffer::begin() noexcept -> iterator { return iterator(_head._next); }
inline auto SlotBuffer::begin() const noexcept -> const_iterator { return cbegin(); }
inline auto SlotBuffer::cbegin() const noexcept -> const_iterator { return const_iterator(_head._next); }

inline auto SlotBuffer::end() noexcept -> iterator { return iterator{&_head}; }
inline auto SlotBuffer::end() const noexcept -> const_iterator { return cend(); }
inline auto SlotBuffer::cend() const noexcept -> const_iterator { return const_iterator{&_head}; }

inline void SlotBuffer::pop_back() { erase(iterator(_head._prev)); }
inline void SlotBuffer::clear() { erase(begin(), end()); collect_garbage(); }

inline void SlotBuffer::splice(const_iterator pos, SlotBuffer & other) { splice(pos, other, other.begin(), other.end()); }  
inline void SlotBuffer::splice(const_iterator pos, SlotBuffer & other, const_iterator it) { splice(pos, other, it, std::next(it)); }  

#if 0
template <typename T>
class sibling_iterator : public SlotBuffer::_iterator<T>
{
public:
    using iterator_category = std::forward_iterator_tag;

    sibling_iterator(SlotBuffer::_iterator<T> &i) 
    : SlotBuffer::_iterator<T>{i}
    {
        decltype(i)::to_cluster_root();
    }

    sibling_iterator<T> &  operator++() noexcept { 
        auto s = SlotBuffer::_iterator<T>::handle();
        assert(s);
        s = s->nextSibling();
        if (s) *this = SlotBuffer::_iterator<T>::from(s);
        else
        {
            while (!static_cast<SlotBuffer::_head_node const *>(s)->is_head_node())
                SlotBuffer::_iterator<T>::operator++();
        }
        return *this; 
    }
    sibling_iterator<T>  operator++(int)  noexcept { decltype(*this) tmp(*this); operator++(); return tmp; }

    SlotBuffer::_iterator<T> &  operator--() noexcept = delete;
    SlotBuffer::_iterator<T>    operator--(int) noexcept = delete;
};
#endif
} // namespace graphite2
