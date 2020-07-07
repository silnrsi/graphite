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

#include "inc/List.h"
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

    class SlotMap;
    enum { MAX_MAPPED = 64 };
private:
    struct _node_linkage {
        _node_linkage * _next;
        _node_linkage * _prev;

        void link(_node_linkage &);
        void unlink();
        void splice(_node_linkage & start, _node_linkage & end);
    };

    template<typename T>
    struct _node : public _node_linkage {
        T _value;
    };

    Vector<_node_linkage *> _slots_storage;
    Vector<int16 *>     _attrs_storage;
    _node_linkage    *  _free_list;
    _node_linkage       _head;
    size_type           _size;
    size_type const     _attrs_size,
                        _chunk_size;

    template<typename T> class _iterator;

    _node<Slot> *   _allocate_node();
    void            _free_node(_node<Slot> * const);
    void            _reverse_range(_node_linkage * const start, _node_linkage * const end);

public:
    using iterator = _iterator<value_type>;
    using const_iterator = _iterator<const value_type>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;


    // TODO: Retire
    iterator newSlot();
    void     freeSlot(iterator i);
    _node_linkage       &head() { return _head; }
    _node_linkage const &head() const { return _head; }

    SlotBuffer(size_t chunk_size = 16, size_t num_user = 32);
//     template<class I>
//     SlotBuffer(size_t num_user, I &start, const I &end);
    ~SlotBuffer();

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

    void push_back(value_type const &);
    void push_back(value_type &&);

    void pop_back();

    iterator erase(iterator first);
    iterator erase(iterator first, iterator last);

    void reverse();
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
    static constexpr S* container_of(void const *p, T S::* m) noexcept {
        return reinterpret_cast<S *>(
                    reinterpret_cast<uint8_t const *>(p) 
                  - reinterpret_cast<uint8_t const *>(&(((S *)0)->*m)));
    }
    
    static _node<T> * node(_iterator<T> const &i) noexcept { 
        return const_cast<_node<T> *>(static_cast<_node<T> const *>(i._p)); 
    }

public:
    using iterator_category = std::bidirectional_iterator_tag;
    using difference_type = ptrdiff_t;
    using value_type = T;
    using pointer = value_type*;
    using reference = value_type&;
    using opaque_type = gr_slot const *;

    // TODO: Remove soon
    auto next() -> decltype(_p) const { return _p->_next; }
    void next(_iterator<T> i) { const_cast<_node_linkage*>(_p)->_next = const_cast<_node_linkage*>(i._p); }

    auto prev() -> decltype(_p) const { return _p->_prev; }
    void prev(_iterator<T> i) { const_cast<_node_linkage*>(_p)->_prev = const_cast<_node_linkage*>(i._p); }

    // TODO
    // static _iterator<T> from(T &r) noexcept { 
    //     return _iterator<T>(container_of<_node<T>>(&r, &_node<T>::_value)); 
    // }
    static _iterator<T> from(T *r) noexcept { 
        return r ? _iterator<T>(container_of<_node<T>>(r, &_node<T>::_value)) : nullptr; 
    }


    void to_cluster_root() noexcept {
        auto p =  operator->();
        while(!p->isBase())  p = p->attachedTo();
        _p = container_of<_node<T>>(p, &_node<T>::_value);
    }

    _iterator(std::nullptr_t): _iterator() {}
    _iterator(_node_linkage const *p)  noexcept: _p{p} {}
    _iterator()  noexcept: _p{nullptr} {}
    _iterator(opaque_type p): _iterator{reinterpret_cast<_node_linkage *>(const_cast<gr_slot*>(p))} {}


    bool operator==(_iterator<T> rhs) const noexcept { return _p == rhs._p; }
    bool operator!=(_iterator<T> rhs) const noexcept { return !operator==(rhs); }

    reference operator*() const noexcept { return node(*this)->_value; }
    pointer operator->() const  noexcept { return &operator*(); }

    _iterator<T> &  operator++()     noexcept { _p = _p->_next; return *this; }
    _iterator<T>    operator++(int)  noexcept { _iterator<T> tmp(*this); operator++(); return tmp; }

    _iterator<T> &  operator--() noexcept    { _p = _p->_prev; return *this; }
    _iterator<T>    operator--(int) noexcept { _iterator<T> tmp(*this); operator--(); return tmp; }

    opaque_type handle() const noexcept { return reinterpret_cast<opaque_type>(_p); }
    
    operator bool() const         noexcept { return _p != nullptr; }
    operator pointer() const      noexcept { return operator->(); }
    operator _iterator<T const>() noexcept { return _iterator<const T>(_p); }
};

inline SlotBuffer::iterator SlotBuffer::newSlot() { auto r = _allocate_node(); r->_next = r->_prev = nullptr; return r; }
inline void                 SlotBuffer::freeSlot(iterator i) { _free_node(iterator::node(i)); }

inline auto SlotBuffer::begin() noexcept -> iterator { return iterator(_head._next); }
inline auto SlotBuffer::begin() const noexcept -> const_iterator { return cbegin(); }
inline auto SlotBuffer::cbegin() const noexcept -> const_iterator { return const_iterator(_head._next); }

inline auto SlotBuffer::end() noexcept -> iterator { return iterator(/*TODO: &_head*/); }
inline auto SlotBuffer::end() const noexcept -> const_iterator { return cend(); }
inline auto SlotBuffer::cend() const noexcept -> const_iterator { return const_iterator(/*TODO: &_head*/); }

inline void SlotBuffer::pop_back() { erase(iterator(_head._prev)); }

} // namespace graphite2




// /*  GRAPHITE2 LICENSING

//     Copyright 2019, SIL International
//     All rights reserved.

//     This library is free software; you can redistribute it and/or modify
//     it under the terms of the GNU Lesser General Public License as published
//     by the Free Software Foundation; either version 2.1 of License, or
//     (at your option) any later version.

//     This program is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//     Lesser General Public License for more details.

//     You should also have received a copy of the GNU Lesser General Public
//     License along with this library in the file named "LICENSE".
//     If not, write to the Free Software Foundation, 51 Franklin Street,
//     Suite 500, Boston, MA 02110-1335, USA or visit their web page on the
//     internet at http://www.fsf.org/licenses/lgpl.html.

// Alternatively, the contents of this file may be used under the terms of the
// Mozilla Public License (http://mozilla.org/MPL) or the GNU General Public
// License, as published by the Free Software Foundation, either version 2
// of the License or (at your option) any later version.
// */
// #pragma once

// #include <iterator>

// #include "inc/List.h"
// #include "inc/Main.h"
// #include "inc/Slot.h"

// #define MAX_SEG_GROWTH_FACTOR  64

// namespace graphite2 {


// class SlotBuffer
// {
//     template <class T>
//     class _iterator
//     {
//     public:
//         using iterator_category = std::bidirectional_iterator_tag;
//         using value_type = T;
//         using difference_type = ptrdiff_t;
//         using pointer = value_type*;
//         using reference = value_type&;

//     private:
//         pointer _p;

//     public:
//         _iterator(pointer p=nullptr): _p(p) {}
        
//         bool operator==(_iterator<T> rhs) const { return _p == rhs._p; }
//         bool operator!=(_iterator<T> rhs) const { return !operator==(rhs); }

//         reference operator*() const { return *_p; }
//         pointer operator->() const { return _p; }

//         _iterator<T> &  operator++() { _p = _p->next(); return *this; }
//         _iterator<T>    operator ++ (int)   { _iterator<T> tmp(*this); operator++(); return tmp; }

//          _iterator<T> &   operator--() { _p = _p->prev(); return *this; }
//         _iterator<T>    operator--(int)   { _iterator<T> tmp(*this); operator--(); return tmp; }

//         pointer ptr() const { return _p; }
//         operator pointer() const { return _p; }
//     };

//     using SlotStore = Vector<Slot *>;
//     using AttributeStore = Vector<int16 *>;

//     // // Prevent copying of any kind.
//     // SlotBuffer& operator=(const SlotBuffer&);

// public:
//     using value_type = Slot;
//     using reference = value_type&;
//     using const_reference = const value_type&;
//     using iterator = _iterator<value_type>;
//     using const_iterator = _iterator<const value_type>;
//     using pointer = value_type*;
//     using const_pointer = const value_type*;
//     using difference_type = ptrdiff_t;
//     using size_type = size_t;

//     SlotBuffer(size_t chunk_size = 16, size_t num_user = 32);
// //     template<class I>
// //     SlotBuffer(size_t num_user, I &start, const I &end);
//     ~SlotBuffer();


// //     size_t size() const { return m_size; }

//     iterator first() { return begin(); }
//     const_iterator first() const { return begin(); }
//     void first(Slot *p) { m_first = p; }
//     iterator last() { return iterator(&back()); }
//     const_iterator last() const { return const_iterator(&back()); }
//     void last(Slot *p) { m_last = p; }
// //     void push_back(const_reference);
//     Slot *newSlot();
//     void freeSlot(pointer);
//     bool grow() const { return !m_free; }

// //     void reverse();

//     CLASS_NEW_DELETE


//     iterator        begin()         { return iterator(m_first); }
//     const_iterator  begin() const   { return cbegin(); }
//     iterator        end()           { return iterator(); }
//     const_iterator  end() const     { return cend(); }
//     const_iterator  cbegin() const  { return const_iterator(m_first); }
//     const_iterator  cend() const    { return const_iterator(); }

//     reference       front()         { return *m_first; }
//     const_reference front() const   { return *m_first; }
//     reference       back()          { return *m_last; }
//     const_reference back() const    { return *m_last; }

// private:
//     // pointer link(pointer pos, reference val);
//     pointer unlink(reference pos);

//     SlotStore       m_slots;        // Vector of slot buffers
//     AttributeStore  m_attrs;        // Vector of userAttrs buffers
//     pointer         m_first,        // first slot in segment
//                     m_last,         // last slot in segment
//                     m_free;         // linked list of free slots
//     size_t          m_size;
//     size_t const    m_attrs_size,   // Per-slot user attributes
//                     m_chunk_size;

// };

// } // namespace graphite2
