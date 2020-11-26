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

// designed to have a limited subset of the std::vector api
#pragma once

#include <cstddef>
#include <cassert>
#include <cstring>
#include <cstdlib>
#include <new>

#include "Main.h"

namespace graphite2 {

template <typename T>
inline
ptrdiff_t distance(T* first, T* last) { return last-first; }


template <typename T>
class vector
{
    T * m_first, *m_last, *m_end;
public:
    using value_type = T;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using reference = T&;
    using const_reference = const T&;
    using iterator = T*;
    using const_iterator = const T*;

    vector() : m_first{nullptr}, m_last{nullptr}, m_end{nullptr} {}
    vector(size_type n, const value_type& value) : vector<T>() { insert(begin(), n, value); }
    explicit vector(size_type n) : vector<T>(n, T()) {}
    template<class It>
    vector(It first, const It last) : vector<T>() { insert(begin(), first, last); }
    vector(const vector<T> &rhs) : vector<T>(rhs.begin(), rhs.end()) { }
    vector(vector<T> &&rhs) : m_first{rhs.m_first}, m_last{rhs.m_last}, m_end{rhs.m_end} { rhs.m_first = rhs.m_last = nullptr; }
    vector(std::initializer_list<T> ilist) : vector{ilist.begin(), ilist.end()} {}
    ~vector() { clear(); free(m_first); }

    iterator            begin()         { return m_first; }
    const_iterator      begin() const   { return m_first; }

    iterator            end()           { return m_last; }
    const_iterator      end() const     { return m_last; }

    bool                empty() const   { return m_first == m_last; }
    size_type           size() const    { return m_last - m_first; }
    size_type           capacity() const{ return m_end - m_first; }

    void                reserve(size_type n);
    void                resize(size_type n, value_type const & v = value_type());

    reference           front()         { assert(size() > 0); return *begin(); }
    const_reference     front() const   { assert(size() > 0); return *begin(); }
    reference           back()          { assert(size() > 0); return *(end()-1); }
    const_reference     back() const    { assert(size() > 0); return *(end()-1); }

    vector<T>         & operator = (const vector<T> & rhs) { if (&rhs != this) assign(rhs.begin(), rhs.end()); return *this; }
    vector<T>         & operator = (vector<T> && rhs);
    reference           operator [] (size_type n)          { assert(size() > n); return m_first[n]; }
    const_reference     operator [] (size_type n) const    { assert(size() > n); return m_first[n]; }

    void                assign(size_type n, const value_type& u)    { clear(); insert(begin(), n, u); }
    template<class It>
    void                assign(It first, It last)      { clear(); insert(begin(), first, last); }
    void                assign(std::initializer_list<T> ilist)  { assign(ilist.begin(), ilist.end()); }
    iterator            insert(iterator p, const value_type & x) { p = _insert_default(p, 1); new (p) value_type(x); return p; }
    void                insert(iterator p, size_type n, const T & x);
    template<class It>
    void                insert(iterator p, It first, It last);
    void                pop_back()              { assert(size() > 0); --m_last; }
    void                push_back(const value_type &v)   { if (m_last == m_end) reserve(size()+1); new (m_last++) value_type(v); }

    template<typename... Args>
    iterator            emplace(iterator p, Args &&... args) { p = _insert_default(p, 1); new (p) value_type(std::forward<Args>(args)...); return p; }
    template<typename... Args>
    reference           emplace_back(Args &&... args) { if (m_last == m_end) reserve(size()+1); return *new (m_last++) value_type(std::forward<Args>(args)...); }

    void                clear()                 { erase(begin(), end()); }
    iterator            erase(iterator p)       { return erase(p, std::next(p)); }
    iterator            erase(iterator first, iterator last);

private:
    iterator            _insert_default(iterator p, size_type n);
};

template <typename T>
inline
void vector<T>::reserve(size_type n)
{
    if (n > capacity())
    {
        const ptrdiff_t sz = size();
        size_t requested;
        if (checked_mul(n, sizeof(value_type), requested))  std::abort();
        m_first = static_cast<value_type*>(realloc(m_first, requested));
        if (!m_first)   std::abort();
        m_last  = m_first + sz;
        m_end   = m_first + n;
    }
}

template <typename T>
inline
void vector<T>::resize(size_type n, const value_type & v) {
    const ptrdiff_t d = n-size();
    if (d < 0)      erase(end()+d, end());
    else if (d > 0) insert(end(), d, v);
}

template<typename T>
inline
vector<T> & vector<T>::operator = (vector<T> && rhs) {
    if (&rhs != this) {
        clear();
        m_first = rhs.m_first;
        m_last  = rhs.m_last;
        m_end   = rhs.m_end;
        rhs.m_first = rhs.m_last = nullptr;
    }
    return *this;
}

template<typename T>
inline
typename vector<T>::iterator vector<T>::_insert_default(iterator p, size_type n)
{
    assert(begin() <= p && p <= end());
    const ptrdiff_t i = p - begin();
    reserve(((size() + n + 7) >> 3) << 3);
    p = begin() + i;
    // Move tail if there is one
    if (p != end()) memmove(p + n, p, distance(p, end())*sizeof(value_type));
    m_last += n;
    return p;
}

template<typename T>
inline
void vector<T>::insert(iterator p, size_type n, const T & x)
{
    p = _insert_default(p, n);
    // Copy in elements
    for (; n; --n, ++p) { new (p) value_type(x); }
}

template<typename T>
template<class It>
inline
void vector<T>::insert(iterator p, It first, It last)
{
    p = _insert_default(p, distance(first, last));
    // Copy in elements
    for (;first != last; ++first, ++p) { new (p) value_type(*first); }
}

template<typename T>
inline
typename vector<T>::iterator vector<T>::erase(iterator first, iterator last)
{
    if (first != last)
    {
        for (iterator e = first; e != last; ++e) e->~value_type();
        auto const sz = distance(first, last);
        if (m_last != last) memmove(first, last, distance(last,end())*sizeof(value_type));
        m_last -= sz;
    }
    return first;
}

} // namespace graphite2
