#ifndef FEATURES_INCLUDE
#define FEATURES_INCLUDE

#include "FeatureMap.h"
#include <stdlib.h>
#include <string.h>

class Features : public IFeatures
{
public:
    virtual void addFeature(FeatureRef &ref, uint16 value) { ref.set(&m_vec, value, m_length); } 
    virtual uint16 getFeature(FeatureRef &ref) { return ref.get(&m_vec, m_length); }
    virtual Features *newCopy() { Features *res = new(m_length) Features(m_length); memcpy(&(res->m_vec), &m_vec, m_length * sizeof(uint32)); return res; }

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
