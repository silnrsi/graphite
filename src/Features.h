#ifndef FEATURES_INCLUDE
#define FEATURES_INCLUDE

#include "FeatureMap.h"
#include <algorithm>

struct Features
{
    virtual void addFeature(FeatureRef *ref, uint16 value) { if (ref) ref->set(m_vec, value, m_length); } 
    virtual uint16 getFeature(FeatureRef *ref) { return ref->get(m_vec, m_length); }
    virtual void addFeatureMask(FeatureRef &ref) { ref.setMask(m_vec, m_length); }
    virtual void maskedOr(Features &other, Features &mask) {
        for (uint32 i = 0; i < m_length; i++)
            if ((&mask.m_vec)[i]) m_vec[i] = (m_vec[i] & ~mask.m_vec[i]) | (other.m_vec[i] & mask.m_vec[i]);
    }

    explicit Features(int num)
      : m_length(num), m_vec(new uint32 [num]) {}
    Features(const Features & o) : m_length(0), m_vec(0) { *this = o; }
    ~Features() { delete [] m_vec; }
    Features & operator=(const Features & rhs) {
        if (m_length != rhs.m_length) {
            delete [] m_vec;
            m_vec = new uint32 [rhs.m_length];
            m_length = m_vec ? rhs.m_length : 0;
        }
        std::copy(rhs.m_vec, rhs.m_vec + m_length, m_vec);
        return *this;
    }
    void setSize(uint32 length) { m_length = length; }
//    void *operator new(size_t dummy, int num) {
//        void *res = malloc(offsetof(Features, m_vec) + num* sizeof(uint32));
//        return res;
//    }
//    void operator delete(void * res, int num) {
//        free(res);
//    }

private:
    uint32 m_length;
    uint32 * m_vec;
};

#endif
