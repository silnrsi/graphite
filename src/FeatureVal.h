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
#pragma once
#include <cstring>
#include <algorithm>
#include <cassert>
#include "Main.h"

class FeatureRef;
class FeatureMap;

class FeatureVal
{
public:
    uint32 maskedOr(const FeatureVal &other, const FeatureVal &mask) {
	uint32 len = m_length ;
	if (other.m_length<len) len = other.m_length;		//defensive
	if (mask.m_length<len) len = mask.m_length;		//defensive
        for (uint32 i = 0; i < len; i++)
            if ((mask.m_vec)[i]) m_vec[i] = (m_vec[i] & ~mask.m_vec[i]) | (other.m_vec[i] & mask.m_vec[i]);
	return len;
    }

    explicit FeatureVal(int num, const FeatureMap* pMap/*not NULL*/)
      : m_length(num), m_vec(gralloc<uint32>(num)), m_pMap(pMap) {}
    FeatureVal() : m_length(0), m_vec(NULL), m_pMap(NULL) { }
    FeatureVal(const FeatureVal & o) : m_length(0), m_vec(0), m_pMap(NULL) { *this = o; }
    ~FeatureVal() { if (m_vec) free(m_vec); }
    FeatureVal & operator=(const FeatureVal & rhs) {
        m_pMap = rhs.m_pMap;
        if (m_length != rhs.m_length) {
            if (m_vec) free(m_vec);
            m_vec = gralloc<uint32>(rhs.m_length);
            m_length = m_vec ? rhs.m_length : 0;
        }
        memmove(m_vec, rhs.m_vec, m_length * sizeof(uint32));
        return *this;
    }
    bool operator ==(const FeatureVal & b) const
    {
        size_t i = 0;
        assert(m_length);
        assert(m_length == b.m_length);
        while (m_vec[i] == b.m_vec[i])
        {
            if (++i == m_length) return true;
        }
        return false;
    }

    void grow(byte reqIndex ) {
        if (reqIndex<m_length) {
          return;
        }
        uint32 * vec = gralloc<uint32>(reqIndex+1);
        memmove(vec, m_vec, m_length * sizeof(uint32));
        memset(vec+m_length, 0, (reqIndex+1-m_length)*sizeof(uint32));
        if (m_vec) free(m_vec);
        m_vec = vec;
        m_length = reqIndex+1;
    }

    FeatureVal* clone() const { return new FeatureVal(*this); }
    
//    void setSize(uint32 length) { m_length = length; }		//unsafe since should also keep m_vec in step

//    void *operator new(size_t dummy, int num) {
//        void *res = malloc(offsetof(Features, m_vec) + num* sizeof(uint32));
//        return res;
//    }
//    void operator delete(void * res, int num) {
//        free(res);
//    }
    CLASS_NEW_DELETE
private:
    friend class FeatureRef;		//so that FeatureRefs can manipulate m_vec directly
    uint32 m_length;
    uint32 * m_vec;
    const FeatureMap* m_pMap;
};

struct gr_feature_val : public FeatureVal {};

typedef FeatureVal Features;
