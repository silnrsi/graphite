#pragma once

#include <algorithm>
#include "Main.h"

namespace org { namespace sil { namespace graphite { namespace v2 {

class FeatureRef;

class Features
{
public:
    uint32 maskedOr(const Features &other, const Features &mask) {
	uint32 len = m_length ;
	if (other.m_length<len) len = other.m_length;		//defensive
	if (mask.m_length<len) len = mask.m_length;		//defensive
        for (uint32 i = 0; i < len; i++)
            if ((mask.m_vec)[i]) m_vec[i] = (m_vec[i] & ~mask.m_vec[i]) | (other.m_vec[i] & mask.m_vec[i]);
	return len;
    }

    explicit Features(int num)
      : m_length(num), m_vec(gralloc<uint32>(num)) {}
    Features(const Features & o) : m_length(0), m_vec(0) { *this = o; }
    ~Features() { free(m_vec); }
    Features & operator=(const Features & rhs) {
        if (m_length != rhs.m_length) {
            if (m_vec) free(m_vec);
            m_vec = gralloc<uint32>(rhs.m_length);
            m_length = m_vec ? rhs.m_length : 0;
        }
        std::copy(rhs.m_vec, rhs.m_vec + m_length, m_vec);
        return *this;
    }
    
    FeaturesHandle clone() const { return new Features(*this); }
    
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
};

}}}} // namespace
