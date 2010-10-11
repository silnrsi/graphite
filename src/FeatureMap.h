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

#include "graphiteng/Types.h"
#include "graphiteng/face.h"
#include "graphiteng/FeaturesHandle.h"
#include "Main.h"
#include "Features.h"
//#include <map> // avoid libstdc++

namespace org { namespace sil { namespace graphite { namespace v2 {

class FeatureRef
{
public:
    FeatureRef(byte bits=0, byte index=0, uint32 mask=0, uint16 flags=0, uint16 uiName=0, uint16 numSet=0, uint16 *uiNames=NULL) throw() 
      : m_mask(mask), m_bits(bits), m_index(index), m_max(mask >> bits),
      m_flags(flags), m_nameid(uiName), m_numSet(numSet), m_nameValues(uiNames) {}
    ~FeatureRef() {
        free(m_nameValues);
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
//     void * operator new (size_t s, FeatureRef * p)
//     {
//         return p;
//     }

    CLASS_NEW_DELETE
private:
    uint32 m_mask;
    uint16 m_max;
    byte m_bits;
    byte m_index;
    uint16 m_nameid;
    uint16 *m_nameValues;
    uint16 m_flags;
    uint16 m_numSet;
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
    
    bool readFace(const void* appFaceHandle/*non-NULL*/, get_table_fn getTable);
    bool readFeats(const void* appFaceHandle/*non-NULL*/, get_table_fn getTable);
    bool readSill(const void* appFaceHandle/*non-NULL*/, get_table_fn getTable);
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
