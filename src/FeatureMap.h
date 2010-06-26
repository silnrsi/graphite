#ifndef FEATUREMAP_INCLUDE
#define FEATUREMAP_INCLUDE

#include "graphiteng/Types.h"
#include "graphiteng/FeaturesHandle.h"
//#include <map> // avoid libstdc++

class IFace;
class Features;

class FeatureRef
{
public:
    FeatureRef(byte bits=0, byte i=0, uint32 mask=0) throw() 
      : m_mask(mask), m_bits(bits), m_index(i), m_max(mask >> bits) {}
    void applyTo(uint32 *vec, uint16 val, byte length) const { 
        if (m_index < length && val <= m_max)
        {
            vec[m_index] &= ~m_mask;
            vec[m_index] |= (val << m_bits);
        }
    }
    uint16 get(const uint32 *vec, byte length) const { if (m_index < length) return (vec[m_index] & m_mask) >> m_bits; else return 0; }
    void applyMaskTo(uint32 *vec, byte length) const { vec[m_index] |= m_mask; }

private:
    uint32 m_mask;
    uint16 m_max;
    byte m_bits;
    byte m_index;
};

class FeatureMap
{
private:
    class LangFeaturePair
    {
    public:
        uint32 m_lang;
        FeaturesHandle m_pFeatures;
    };
public:
    FeatureMap() : m_langFeats(NULL), m_feats(NULL) {}
    ~FeatureMap() { delete[] m_langFeats; delete[] m_feats; }
    
    bool readFont(const IFace *face);
    bool readFeats(const IFace *face);
    bool readSill(const IFace *face);
    const FeatureRef *featureRef(uint32 name);
    FeatureRef *feature(uint8 index) const { return m_feats + index; }
    FeatureRef *ref(byte index) { return index < m_numFeats ? m_feats + index : NULL; }
    FeaturesHandle cloneFeatures(uint32 langname/*0 means default*/) const;

private:
    byte m_numFeats;
//    std::map<uint32, byte> m_map;
//    std::map<uint32, Features *>m_langMap;
    LangFeaturePair * m_langFeats;
    uint16 m_numLanguages;

    FeatureRef *m_feats;
    FeaturesHandle m_defaultFeatures;
    
private:		//defensive on m_langFeats and m_feats
    FeatureMap(const FeatureMap&);
    FeatureMap& operator=(const FeatureMap&);
};

#endif
