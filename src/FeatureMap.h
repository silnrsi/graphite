#ifndef FEATUREMAP_INCLUDE
#define FEATUREMAP_INCLUDE

#include <map>
#include "graphiteng/Types.h"
#include "graphiteng/IFeatures.h"

class IFace;

class FeatureRef
{
public:
    void set(uint32 *vec, uint16 val, byte length) { 
        if (m_index < length && val <= m_max)
        {
            vec[m_index] &= ~m_mask;
            vec[m_index] |= (val << m_bits);
        }
    }
    uint16 get(uint32 *vec, byte length) { if (m_index < length) return (vec[m_index] & m_mask) >> m_bits; else return 0; }
    void setMask(uint32 *vec, byte length) { vec[m_index] |= m_mask; }
    void init(byte bits, byte i, uint32 mask) { m_mask = mask; m_bits = bits; m_index = i; }

protected:
    uint32 m_mask;
    uint16 m_max;
    byte m_bits;
    byte m_index;
};

class FeatureMap
{
public:
    bool readFont(IFace *face);
    bool readFeats(IFace *face);
    bool readSill(IFace *face);
    FeatureRef *featureRef(uint32 name);
    FeatureRef *ref(byte index) { return index < m_numFeats ? m_feats + index : NULL; }
    IFeatures *newFeatures(uint32 name);

protected:
    byte m_numFeats;
    std::map<uint32, byte> m_map;
    std::map<uint32, IFeatures *>m_langMap;
    FeatureRef *m_feats;
    IFeatures *m_defaultFeatures;
};

#endif
