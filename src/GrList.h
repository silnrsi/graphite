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

// designed to have a limited subset of the std::vector api

namespace org { namespace sil { namespace graphite { namespace v2 {

template <class T> class GrList;
template <class T> class GrListUnitIterator;

template <class T>
class GrListUnit
{
    GrListUnit() : m_previous(NULL), m_next(NULL) {}
    friend class GrList<T>;
    friend class GrListUnitIterator<T>;
    T m_value;
    GrListUnit<T> * m_previous;
    GrListUnit<T> * m_next;
public:
    CLASS_NEW_DELETE
};

template <class T>
class GrListUnitIterator
{
public:
    GrListUnitIterator(const GrList<T> & list, GrListUnit<T> * unit)
    : m_list(list), m_unit(unit) {}
    GrListUnitIterator(const GrListUnitIterator<T> & toCopy)
    : m_list(toCopy.m_list), m_unit(toCopy.m_unit) {}
    GrListUnitIterator<T> operator+(int value)
    {
        if (m_list.m_sequential)
        {
            m_unit+= value;
            assert(m_unit <= m_list.m_begin + m_list.m_blockSize);
            if (m_unit == m_list.m_begin + m_list.m_blockSize)
                m_unit = NULL;
            return *this;
        }
        if (m_unit == m_list.m_begin)
        {
            m_unit = m_list.get(value);
        }
        else if (m_unit == m_list.m_lastAccess)
        {
            m_unit = m_list.get(m_list.m_lastAccessIndex + value);
        }
        else
        {
            while (value > 0 && m_unit)
            {
                assert(m_unit->m_next);
                m_unit = m_unit->m_next;
                --value;
            }
            while (value < 0 && m_unit)
            {
                assert(m_unit->m_previous);
                m_unit = m_unit->m_previous;
                ++value;
            }
        }
        return *this;
    }
    GrListUnitIterator< T > operator += (int value)
    {
        operator +(value);
        return *this;
    }
    GrListUnitIterator<T> operator-(int value)
    {
        if (m_list.m_sequential)
        {
            m_unit-= value;
            assert(m_list.m_begin <= m_unit);
            assert(m_unit <= m_list.m_begin + m_list.m_blockSize);
            if (m_unit == m_list.m_begin + m_list.m_blockSize)
                m_unit = NULL;
            return *this;
        }
        if (m_unit == m_list.m_begin)
        {
            m_unit = m_list.get(value);
        }
        else if (m_unit == m_list.m_lastAccess)
        {
            m_unit = m_list.get(m_list.m_lastAccessIndex - value);
        }
        else
        {
            while (value < 0 && m_unit)
            {
                assert(m_unit->m_next);
                m_unit = m_unit->m_next;
                ++value;
            }
            while (value > 0 && m_unit)
            {
                assert(m_unit->m_previous);
                m_unit = m_unit->m_previous;
                --value;
            }
        }
        return *this;
    }
    bool operator == (const GrListUnitIterator<T> & i) const
    {
        assert(i.m_list == m_list); // check parent list is same
        return m_unit == i.m_unit;
    }
    bool operator != (const GrListUnitIterator<T> & i) const
    {
        assert(i.m_list.m_pBlock == m_list.m_pBlock); // check parent list is same
        return m_unit != i.m_unit;
    }
protected:
    friend class GrList<T>;
    const GrList< T > & m_list;
    GrListUnit<T> * m_unit;

};

// A linked list implmentation which allocates memory in blocks

template <class T>
class GrList
{
protected:
    friend class GrListUnitIterator<T>;
public:
    typedef GrListUnitIterator<T> iterator;
    GrList(size_t suggestedSize) :
        m_lastAccessIndex(0),
        m_length(0),
        m_blockSize(suggestedSize),
        m_numBlocks(1),
        m_currentBlock(0),
        m_back(NULL),
        m_sequential(true)
    {
        for (size_t i = 0; i < MAX_EXPANSION; i++) m_pBlock[i] = NULL;
        m_pBlock[0] = new GrListUnit<T>[m_blockSize];
//        m_pBlock[0] = gralloc<GrListUnit<T> >(m_blockSize + 1);
        m_firstUnallocated = m_pBlock[0];
        m_lastAccess = m_begin = NULL;
        T value;
        for (size_t i = 0; i < suggestedSize; i++)
        {
            push_back(value);
        }
    }
    GrList(size_t suggestedSize, const T & value) :
        m_lastAccessIndex(0),
        m_length(0),
        m_blockSize(suggestedSize),
        m_numBlocks(1),
        m_currentBlock(0),
        m_back(NULL),
        m_sequential(true)
    {
        for (size_t i = 0; i < MAX_EXPANSION; i++) m_pBlock[i] = NULL;
        //m_pBlock[0] = gralloc<GrListUnit<T> >(m_blockSize + 1);
        m_pBlock[0] = new GrListUnit<T>[m_blockSize];
        m_firstUnallocated = m_pBlock[0];
        m_lastAccess = m_begin = NULL;
        for (size_t i = 0; i < suggestedSize; i++)
        {
            push_back(value);
        }
    }
    GrList() :
        m_lastAccessIndex(0),
        m_length(0),
        m_blockSize(0),
        m_numBlocks(0),
        m_currentBlock(0),
        m_begin(NULL),
        m_back(NULL),
        m_firstUnallocated(NULL),
        m_lastAccess(NULL),
        m_sequential(true)
    {
        for (size_t i = 0; i < MAX_EXPANSION; i++) m_pBlock[i] = NULL;
        
    }        
    ~GrList()
    {
        for (size_t i = 0; i < MAX_EXPANSION; i++)
        {
            if (m_pBlock[i])
            {
                //free(m_pBlock[i]);
                delete [] m_pBlock[i];
                m_pBlock[i] = NULL;
            }
        }
    }
    // fast for n close to m_lastAccessIndex, otherwise order(m_length)
    GrListUnit<T> * get(size_t n) const
    {
        if (n == m_length) return NULL;
        if (m_sequential)
        {
            return m_begin + n;
        }
        assert(n < m_length);
        if (m_lastAccessIndex == n)
        {
            return m_lastAccess;
        }
        else if (m_lastAccessIndex < n) // go forwards
        {
            if (n >= m_length)
            {
                return NULL;
            }
            do
            {
                m_lastAccess = m_lastAccess->m_next;
                assert(m_lastAccess);
                ++m_lastAccessIndex;
            } while (m_lastAccessIndex < n);
        }
        else
        {
            if (n < (m_lastAccessIndex >> 1))
            {
                // start from beginning
                m_lastAccess = m_begin;
                m_lastAccessIndex = 0;
                while (n > m_lastAccessIndex)
                {
                    m_lastAccess = m_lastAccess->m_next;
                    ++m_lastAccessIndex;
                }
            }
            else // go backwards
            {
                do
                {
                    m_lastAccess = m_lastAccess->m_previous;
                    assert(m_lastAccess);
                    --m_lastAccessIndex;
                } while (m_lastAccessIndex > n);
            }
        }
        return m_lastAccess;
    }
    T & operator[](size_t n)
    {
        return get(n)->m_value;
//        GrListUnit<T> * p = get(n);
//        assert(p);
//        return p->m_value;
    }
    const T & operator[](size_t n) const
    {
        return get(n)->m_value;
//        GrListUnit<T> * p = get(n);
//        assert(p);
//        return p->m_value;
    }
    const iterator begin() const { return GrListUnitIterator<T>(*this, m_begin); }
    const iterator end() const { return GrListUnitIterator<T>(*this, NULL); }
    iterator begin() { return GrListUnitIterator<T>(*this, m_begin); }
    iterator end() { return GrListUnitIterator<T>(*this, NULL); }
    bool empty() const { return m_length == 0; }
    size_t size() const { return m_length; }

    void push_back(const T& data)
    {
        if (m_blockSize == 0) m_blockSize = 16;
        allocIfNeeded();
        assert(m_firstUnallocated);
        m_firstUnallocated->m_value = data;
        if (m_back)
        {
            assert(m_back->m_next == NULL);
            m_back->m_next = m_firstUnallocated;
        }
        else
        {
            m_begin = m_firstUnallocated;
            m_lastAccess = m_begin;
            m_lastAccessIndex = 0;
        }
        m_firstUnallocated->m_previous = m_back;
        m_firstUnallocated->m_next = NULL;
        m_back = m_firstUnallocated;
        nextUnallocated();
        
        ++m_length;
    }
    void assign(size_t n, const T & value)
    {
        for (size_t i = 0; i < m_numBlocks; i++)
        {
            if (m_pBlock[i])
            {
                //free(m_pBlock[i]);
                delete [] m_pBlock[i];
                m_pBlock[i] = NULL;
            }
            m_sequential = true;
        }
        m_blockSize = n;
        //m_pBlock[0] = gralloc<GrListUnit<T> >(m_blockSize);
        m_pBlock[0] = new GrListUnit<T>[m_blockSize];
        m_firstUnallocated = m_pBlock[0];
        m_lastAccess = m_begin = NULL;
        m_numBlocks = 1;
        for (size_t i = 0; i < n; i++)
        {
            push_back(value);
        }
    }
    void reserve(size_t n)
    {
        if (m_pBlock[0])
        {
            while (m_numBlocks * m_blockSize < n && m_numBlocks + 1 < MAX_EXPANSION)
            {
                //m_pBlock[m_numBlocks++] = gralloc<GrListUnit<T> >(m_blockSize);
                m_pBlock[m_numBlocks++] = new GrListUnit<T>[m_blockSize];
            }
            assert(m_numBlocks * m_blockSize >= n);
        }
        else // set the block size afresh
        {
            m_blockSize = n;
            //m_pBlock[0] = gralloc<GrListUnit<T> >(m_blockSize);
            m_pBlock[0] = new GrListUnit<T>[m_blockSize];
            m_firstUnallocated = m_pBlock[0];
            m_lastAccess = m_begin = NULL;
            m_numBlocks = 1;
            m_currentBlock = 0;
        }
    }
    iterator insert(iterator i, const T & t)
    {
        if (i.m_unit) m_sequential = false;
        allocIfNeeded();
        assert(m_firstUnallocated);
        if (m_lastAccess == i.m_unit)
        {
            m_lastAccess = m_firstUnallocated;
        }
        else if (i.m_unit && m_lastAccess == i.m_unit->m_previous)
        {
        }
        else if (i.m_unit && m_lastAccess == i.m_unit->m_next)
        {
            ++m_lastAccessIndex;
        }
        else
        {
            // unknown reset to start
            m_lastAccess = m_begin;
            m_lastAccessIndex = 0;
        }
        if (i.m_unit)
        {
            m_firstUnallocated->m_previous = i.m_unit->m_previous;
            i.m_unit->m_previous = m_firstUnallocated;
        }
        else
        {
            m_firstUnallocated->m_previous = m_back;
            m_back = m_firstUnallocated;
        }
        if (m_firstUnallocated->m_previous)
        {
            m_firstUnallocated->m_previous->m_next = m_firstUnallocated;
        }
        else
        {
            m_begin = m_firstUnallocated;
        }
        m_firstUnallocated->m_next = i.m_unit;
        m_firstUnallocated->m_value = t;
        // return inserted value
        i.m_unit = m_firstUnallocated;
        nextUnallocated();
        ++m_length;
        return i;
    }
    void insert(iterator i, size_t n, const T& x)
    {
        if (i.m_unit) m_sequential = false;
        allocIfNeeded();
        assert(m_firstUnallocated);
        if (m_lastAccess == i.m_unit)
        {
            m_lastAccess = m_firstUnallocated;
        }
        else if (i.m_unit && m_lastAccess == i.m_unit->m_previous)
        {
        }
        else if (i.m_unit && m_lastAccess == i.m_unit->m_next)
        {
            m_lastAccessIndex += n;
        }
        else
        {
            // unknown reset to start
            m_lastAccess = m_begin;
            m_lastAccessIndex = 0;
        }
        GrListUnit<T>* previous = (i.m_unit)? i.m_unit->m_previous : m_back;
        m_length += n;
        for (; n > 0; --n)
        {
            allocIfNeeded();
            m_firstUnallocated->m_previous = previous;
            if (previous)
            {
                previous->m_next = m_firstUnallocated;
            }
            else
            {
                m_begin = m_firstUnallocated;
            }
            m_firstUnallocated->m_value = x;
            previous = m_firstUnallocated;
            nextUnallocated();
        }
        if (i.m_unit)
        {
            i.m_unit->m_previous = previous;
            previous->m_next = i.m_unit;
        }
        else
        {
            previous->m_next = NULL;
            m_back = previous;
        }
    }
    void insert(iterator i, iterator f, iterator l)
    {
        if (i.m_unit) m_sequential = false;
        allocIfNeeded();
        assert(m_firstUnallocated);
        if (m_lastAccess == i.m_unit)
        {
            m_lastAccess = m_firstUnallocated;
        }
        else if (i.m_unit && m_lastAccess == i.m_unit->m_previous)
        {
        }
        else
        {
            // unknown reset to start
            m_lastAccess = m_begin;
            m_lastAccessIndex = 0;
        }
        GrListUnit<T>* previous = (i.m_unit)? i.m_unit->m_previous : m_back;
        while (f != l)
        {
            ++m_length;
            allocIfNeeded();
            m_firstUnallocated->m_previous = previous;
            if (previous)
            {
                previous->m_next = m_firstUnallocated;
            }
            else
            {
                m_begin = m_firstUnallocated;
                m_lastAccess = m_begin;
                m_lastAccessIndex = 0;
            }
            m_firstUnallocated->m_value = f.m_unit->m_value;
            previous = m_firstUnallocated;
            nextUnallocated();
            f += 1;
        }
        if (i.m_unit)
        {
            i.m_unit->m_previous = previous;
            previous->m_next = i.m_unit;
        }
        else
        {
            previous->m_next = NULL;
            m_back = previous;
        }
    }
    iterator erase(iterator i)
    {
        m_sequential = false;
        --m_length;
        if (i.m_unit->m_next)
        {
            i.m_unit->m_next->m_previous = i.m_unit->m_previous;
        }
        else
        {
            assert(m_back == i.m_unit);
            m_back = m_back->m_previous;
        }
        if (i.m_unit->m_previous)
        {
            i.m_unit->m_previous->m_next = i.m_unit->m_next;
        }
        else
        {
            assert(i.m_unit == m_begin);
            if (m_length == 0)
            {
                // reset everything
                reset();
                i.m_unit = NULL;
                return i;
            }
            else
            {
                m_begin = i.m_unit->m_next;
            }
        }

        if (m_lastAccess == i.m_unit)
        {
            m_lastAccess = i.m_unit->m_next;
            if (!m_lastAccess)
            {
                m_lastAccess = i.m_unit->m_previous;
                --m_lastAccessIndex;
            }
        }
        else if (m_lastAccess == i.m_unit->m_next)
        {
            --m_lastAccessIndex;
        }
        else if (m_lastAccess == i.m_unit->m_previous)
        {
            // nothing to do
        }
        else // could be anywhere, reset to start
        {
            // this could give poor performance
            m_lastAccess = m_begin;
            m_lastAccessIndex = 0;
        }
        GrListUnit<T> * temp = i.m_unit->m_next;
        i.m_unit->m_previous = NULL;
        i.m_unit->m_next = NULL;
        i.m_unit = temp;
        return i;
    }
    iterator erase(iterator p, iterator q)
    {
        m_sequential = false;
        if (m_lastAccess == q.m_unit)
        {
            // adjust the index
            GrListUnit<T> * temp = q.m_unit;
            while (temp != p.m_unit)
            {
                --m_lastAccessIndex;
                --m_length;
                temp = temp->m_previous;
                assert(temp->m_previous == NULL || temp->m_previous->m_next == temp);
                temp->m_next->m_previous = NULL;
                temp->m_next = NULL;
            }
        }
        else if (q.m_unit == NULL)
        {
            // q is end of list
            GrListUnit<T> * temp = p.m_unit;
            do
            {
                --m_length;
                temp = temp->m_next;
                if (temp)
                {
                    if (temp->m_previous)
                        temp->m_previous->m_next = NULL;
                    temp->m_previous = NULL;
                }
            } while (temp);
            if (m_lastAccess == p.m_unit)
            {
                m_lastAccess = p.m_unit->m_previous;
                --m_lastAccessIndex;
            }
            else if (m_lastAccess == p.m_unit->m_previous)
            {
                // nothing to do
            }
            else
            {
                m_lastAccess = m_begin;
                m_lastAccessIndex = 0;
            }
        }
        else
        {
            if (m_lastAccess == p.m_unit)
            {
                m_lastAccess = q.m_unit;
            }
            else if (m_lastAccess == p.m_unit->m_previous)
            {
                // nothing to do
            }
            else // could be anywhere, reset to start
            {
                // this could give poor performance
                m_lastAccess = m_begin;
                m_lastAccessIndex = 0;
            }
            GrListUnit<T> * temp = q.m_unit;
            while (temp != p.m_unit)
            {
                --m_length;
                assert(temp);
                temp = temp->m_previous;
                assert(temp->m_previous == NULL || temp->m_previous->m_next == temp);
                temp->m_next->m_previous = NULL;
                temp->m_next = NULL;
            }
        }
        
        if (q.m_unit)
        {
            q.m_unit->m_previous = p.m_unit->m_previous;
        }
        else
        {
            m_back = p.m_unit->m_previous;
        }
        if (p.m_unit->m_previous)
        {
            p.m_unit->m_previous->m_next = q.m_unit;
        }
        else
        {
            if (m_length > 0)
            {
                m_begin = q.m_unit;
            }
            else
            {
                // reset, everything erased
                reset();
                return q;
            }
        }

        GrListUnit<T> * temp = q.m_unit;
        p.m_unit->m_previous = NULL;
        p.m_unit->m_next = NULL;
        p.m_unit = temp;
        return p;
    }
    void clear()
    {
        m_length = 0;
        reset();
    }
private:
    enum
    {
        // allow the list to expand to up to 8 times its original size
        MAX_EXPANSION = 8
    };
    void allocIfNeeded()
    {
        if (!m_firstUnallocated)
        {
            assert(m_numBlocks >= m_currentBlock);
            assert(m_numBlocks < MAX_EXPANSION);
            m_sequential = false;
            m_pBlock[m_numBlocks] = new GrListUnit<T>[m_blockSize];
            //gralloc<GrListUnit<T> >(m_blockSize);
            m_firstUnallocated = m_pBlock[m_numBlocks];
            m_currentBlock = m_numBlocks;
            m_numBlocks++;
        }
    }
    void nextUnallocated()
    {
        if (static_cast<size_t>(++m_firstUnallocated - m_pBlock[m_currentBlock]) >= m_blockSize)
        {
            m_firstUnallocated = m_pBlock[++m_currentBlock];
        }
    }
    /*
    void checkSize(size_t n)
    {
        size_t newBlocks = n / m_blockSize;
        if (newBlocks > m_numBlocks)
        {
            new GrListUnit<T>[m_blockSize];
            gralloc<GrListUnit<T> >(m_blockSize);
        }
    }*/
    void reset()
    {
        m_firstUnallocated = m_pBlock[0];
        m_lastAccess = m_begin = m_back = NULL;
        m_currentBlock = 0;
        m_lastAccessIndex = 0;
    }
    mutable size_t m_lastAccessIndex;
    size_t m_length;
    size_t m_blockSize;
    unsigned short m_numBlocks;
    unsigned short m_currentBlock;
    GrListUnit<T> * m_pBlock[MAX_EXPANSION];
    GrListUnit<T> * m_begin;
    GrListUnit<T> * m_back; // end is 1 past this
    GrListUnit<T> * m_firstUnallocated; // 1 past allocated
    mutable GrListUnit<T> * m_lastAccess;
    bool m_sequential;
};


}}}}
