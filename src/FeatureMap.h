#pragma once

#include "graphiteng/Types.h"
#include "graphiteng/FeaturesHandle.h"
#include "Main.h"
#include "Features.h"
//#include <map> // avoid libstdc++

namespace org { namespace sil { namespace graphite { namespace v2 {

class IFace;

class FeatureRef
{
public:
    FeatureRef(byte bits=0, byte index=0, uint32 mask=0) throw() 
      : m_mask(mask), m_bits(bits), m_index(index), m_max(mask >> bits) {}
    void applyValToFeature(uint16 val, Features* pDest) const { 
        if (m_index < pDest->m_length && val <= m_max)
        {
            pDest->m_vec[m_index] &= ~m_mask;
            pDest->m_vec[m_index] |= (val << m_bits);
        }
    }
    void maskFeature(Features* pDest) const { 
	if (m_index < pDest->m_length) 				//defensive
	    pDest->m_vec[m_index] |= m_mask; 
    }

    uint16 getFeatureVal(const Features& feats) const { 
	if (m_index < feats.m_length) 
	    return (feats.m_vec[m_index] & m_mask) >> m_bits; 
	else 
	    return 0; 
    }
    void * operator new (size_t s, FeatureRef * p)
    {
        return p;
    }

    CLASS_NEW_DELETE
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
        CLASS_NEW_DELETE
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
    CLASS_NEW_DELETE
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

}}}} // namespace
