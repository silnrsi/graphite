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
#include "graphiteng/Types.h"
#include "graphiteng/GrFace.h"
#include "graphiteng/Features.h"
#include "Main.h"
#include "FeaturesImp.h"
//#include <map> // avoid libstdc++

namespace org { namespace sil { namespace graphite { namespace v2 {

class FeatureSetting
{
public:
    FeatureSetting(uint16 labelId, int16 theValue) : m_label(labelId), m_value(theValue) {};
    FeatureSetting(const FeatureSetting & fs) : m_label(fs.m_label), m_value(fs.m_value) {};
    uint16 label() const { return m_label; }
    int16 value() const { return m_value; }
private:
    uint16 m_label;
    int16 m_value;
};

class FeatureRef
{
public:
    FeatureRef(byte bits=0, byte index=0, uint32 mask=0, uint16 flags=0,
               uint32 name=0, uint16 uiName=0, uint16 numSet=0, FeatureSetting *uiNames=NULL) throw()
      : m_mask(mask), m_id(name), m_bits(bits), m_index(index), m_max(mask >> bits),
      m_flags(flags), m_nameid(uiName), m_numSet(numSet), m_nameValues(uiNames) {}
    FeatureRef(const FeatureRef & toCopy)
        : m_mask(toCopy.m_mask), m_id(toCopy.m_id), m_bits(toCopy.m_bits),
        m_index(toCopy.m_index), m_max(toCopy.m_max), m_flags(toCopy.m_flags),
        m_nameid(toCopy.m_nameid), m_numSet(toCopy.m_numSet),
        m_nameValues((toCopy.m_nameValues)? gralloc<FeatureSetting>(toCopy.m_numSet) : NULL)
    {
        // most of the time these name values aren't used, so NULL might be acceptable
        if (toCopy.m_nameValues)
        {
            memcpy(m_nameValues, toCopy.m_nameValues, sizeof(FeatureSetting) * m_numSet);
        }
    }
    ~FeatureRef() {
        if (m_nameValues) free(m_nameValues);
        m_nameValues = NULL;
    }
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

    uint32 getId() const { return m_id; }
    uint16 getNameId() const { return m_nameid; }
    uint16 getNumSettings() const { return m_numSet; }
    uint16 getSettingName(uint16 index) const { return m_nameValues[index].label(); }
    int16 getSettingValue(uint16 index) const { return m_nameValues[index].value(); }

//     void * operator new (size_t s, FeatureRef * p)
//     {
//         return p;
//     }

    CLASS_NEW_DELETE
private:
    uint32 m_mask;              // bit mask to get the value from the vector
    uint32 m_id;                // feature identifier/name
    uint16 m_max;               // max value the value can take
    byte m_bits;                // how many bits to shift the value into place
    byte m_index;               // index into the array to find the ulong to mask
    uint16 m_nameid;            // Name table id for feature name
    FeatureSetting *m_nameValues;       // array of name table ids for feature values
    uint16 m_flags;             // feature flags (unused at the moment but read from the font)
    uint16 m_numSet;            // number of values (number of entries in m_nameValues)
};

class FeatureMap
{
private:
    class LangFeaturePair
    {
    public:
        uint32 m_lang;
        Features* m_pFeatures;      //owns
        CLASS_NEW_DELETE
    };
public:
    FeatureMap() : m_numFeats(0), m_numLanguages(0), m_searchIndex(0),
        m_sortedIndexes(NULL), m_langFeats(NULL),
        m_feats(NULL), m_defaultFeatures(NULL) {}
    ~FeatureMap() { delete[] m_langFeats; delete[] m_feats; delete m_defaultFeatures; }
    
    bool readFace(const void* appFaceHandle/*non-NULL*/, get_table_fn getTable);
    bool readFeats(const void* appFaceHandle/*non-NULL*/, get_table_fn getTable);
    bool readSill(const void* appFaceHandle/*non-NULL*/, get_table_fn getTable);
    bool createSortedFeatureList(); // public for testing purposes
    const FeatureRef *featureRef(uint32 name) const;
    FeatureRef *feature(uint8 index) const { return m_feats + index; }
    FeatureRef *ref(byte index) { return index < m_numFeats ? m_feats + index : NULL; }
    Features* cloneFeatures(uint32 langname/*0 means default*/) const;      //call destroy_Features when done.
    uint16 numFeatures() const { return m_numFeats; };
    uint16 numLanguages() const { return m_numLanguages; };
    uint32 getLangName(uint16 index) const { return (index < m_numLanguages)? m_langFeats[index].m_lang : 0; };
    CLASS_NEW_DELETE
private:

    uint16 m_numFeats;
    uint16 m_numLanguages;
    uint16 m_searchIndex;
    uint16 * m_sortedIndexes;
    LangFeaturePair * m_langFeats;

    FeatureRef *m_feats;
    Features* m_defaultFeatures;        //owned

private: //defensive on m_langFeats and m_feats
    FeatureMap(const FeatureMap&);
    FeatureMap& operator=(const FeatureMap&);
};

}}}} // namespace
