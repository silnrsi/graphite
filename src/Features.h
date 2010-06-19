#ifndef FEATURES_INCLUDE
#define FEATURES_INCLUDE

#include "FeatureMap.h"
#include <stdlib.h>
#include <string.h>

class Features : public IFeatures
{
public:
    virtual void addFeature(FeatureRef *ref, uint16 value) { if (ref) ref->set(&m_vec, value, m_length); } 
    virtual uint16 getFeature(FeatureRef &ref) { return ref.get(&m_vec, m_length); }
    virtual void addFeatureMask(FeatureRef &ref) { ref.setMask(&m_vec, m_length); }
    virtual Features *newCopy() { Features *res = new(m_length) Features(m_length); memcpy(&(res->m_vec), &m_vec, m_length * sizeof(uint32)); return res; }
    virtual void maskedOr(Features &other, Features &mask) {
        for (uint32 i = 0; i < m_length; i++)
            if ((&mask.m_vec)[i]) (&m_vec)[i] = ((&m_vec)[i] & ~(&mask.m_vec)[i]) | ((&other.m_vec)[i] & (&mask.m_vec)[i]);
    }

    Features(int num) : m_length(num) { }
    void setSize(uint32 length) { m_length = length; }
    void *operator new(size_t dummy, int num) {
        void *res = malloc((num - 1) * sizeof(uint32) + sizeof(Features));
        memset(res, 0, sizeof(Features) + (num - 1) * sizeof(uint32));
        return res;
    }

protected:
    uint32 m_length;
    uint32 m_vec;
};

#endif
