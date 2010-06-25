#ifndef FEATUREMAP_INCLUDE
#define FEATUREMAP_INCLUDE

#include <map>
#include "graphiteng/Types.h"

class IFace;
class Features;

class FeatureRef
{
public:
    FeatureRef(byte bits=0, byte i=0, uint32 mask=0) throw() 
      : m_mask(mask), m_bits(bits), m_index(i), m_max(mask >> bits) {}
    void set(uint32 *vec, uint16 val, byte length) { 
        if (m_index < length && val <= m_max)
        {
            vec[m_index] &= ~m_mask;
            vec[m_index] |= (val << m_bits);
        }
    }
    uint16 get(uint32 *vec, byte length) { if (m_index < length) return (vec[m_index] & m_mask) >> m_bits; else return 0; }
    void setMask(uint32 *vec, byte length) { vec[m_index] |= m_mask; }

protected:
    uint32 m_mask;
    uint16 m_max;
    byte m_bits;
    byte m_index;
};

class FeatureMap
{
public:
    bool readFont(const IFace *face);
    bool readFeats(const IFace *face);
    bool readSill(const IFace *face);
    FeatureRef *featureRef(uint32 name);
    FeatureRef *feature(uint8 index) { return m_feats + index; }
    FeatureRef *ref(byte index) { return index < m_numFeats ? m_feats + index : NULL; }
    Features *newFeatures(uint32 name) const;

protected:
    byte m_numFeats;
    std::map<uint32, byte> m_map;
    std::map<uint32, Features *>m_langMap;
    FeatureRef *m_feats;
    Features *m_defaultFeatures;
};

#endif
