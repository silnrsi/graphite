#ifndef FEATURES_INCLUDE
#define FEATURES_INCLUDE

#include "FeatureMap.h"
#include <algorithm>

class Features
{
public:
    void addFeature(const FeatureRef *ref, uint16 value) { if (ref) ref->applyTo(m_vec, value, m_length); } 
    uint16 getFeature(const FeatureRef *ref) const { return ref->get(m_vec, m_length); }
    void addFeatureMask(const FeatureRef &ref) { ref.applyMaskTo(m_vec, m_length); }
    void maskedOr(const Features &other, const Features &mask) {
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
    
    FeaturesHandle clone() const { return new Features(*this); }
    
//    void setSize(uint32 length) { m_length = length; }		//unsafe since should also keep m_vec in step

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

private :
#ifdef FIND_BROKEN_VIRTUALS
    virtual double addFeature(FeatureRef *ref, uint16 value) { return 0.0; } 
    virtual double getFeature(FeatureRef *ref) { return 0.0; }
    virtual double addFeatureMask(FeatureRef &ref) { return 0.0; }
    virtual double maskedOr(Features &other, Features &mask) { return 0.0; }
#endif		//FIND_BROKEN_VIRTUALS
};

#endif
