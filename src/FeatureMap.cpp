#include <string.h>

#include "Main.h"
#include "FeatureMap.h"
#include "Features.h"
#include "graphiteng/IFace.h"
#include "XmlTraceLog.h"
#include "TtfUtil.h"

#define ktiFeat MAKE_TAG('F','e','a','t')
#define ktiSill MAKE_TAG('S','i','l','l')



bool FeatureMap::readFont(const IFace *face)
{
    if (!readFeats(face)) return false;
    if (!readSill(face)) return false;
    return true;
}

bool FeatureMap::readFeats(const IFace *face)
{
    size_t lFeat;
    char *pFeat = (char *)(face->getTable(ktiFeat, &lFeat));
    char *pOrig = pFeat;
    uint16 *defVals;
    uint32 version;
    if (!pFeat) return true;
    if (lFeat < 12) return false;

    version = read32(pFeat);
    if (version < 0x00010000) return false;
    m_numFeats = read16(pFeat);
    read16(pFeat);
    read32(pFeat);
    if (m_numFeats * 16U + 12 > lFeat) { m_numFeats = 0; return false; }		//defensive
    m_feats = new FeatureRef[m_numFeats];
    defVals = new uint16[m_numFeats];
    byte currIndex = 0;
    byte currBits = 0;

#ifndef DISABLE_TRACING
    XmlTraceLog::get().openElement(ElementFeatures);
    XmlTraceLog::get().addAttribute(AttrMajor, version >> 16);
    XmlTraceLog::get().addAttribute(AttrMinor, version & 0xFFFF);
    XmlTraceLog::get().addAttribute(AttrNum, m_numFeats);
#endif
    for (int i = 0; i < m_numFeats; i++)
    {
        uint32 name;
        if (version < 0x00020000)
            name = read16(pFeat);
        else
            name = read32(pFeat);
        uint16 numSet = read16(pFeat);
        uint32 offset;
        if (version < 0x00020000)
            offset = read32(pFeat);
        else
        {
            read16(pFeat);
            offset = read32(pFeat);
        }
        uint16 flags = read16(pFeat);
        uint16 uiName = read16(pFeat);
        char *pSet = pOrig + offset;
        uint16 maxVal = 0;

#ifndef DISABLE_TRACING
        XmlTraceLog::get().openElement(ElementFeature);
        XmlTraceLog::get().addAttribute(AttrIndex, i);
        XmlTraceLog::get().addAttribute(AttrNum, name);
        XmlTraceLog::get().addAttribute(AttrFlags, flags);
        XmlTraceLog::get().addAttribute(AttrLabel, uiName);
#endif
        if (offset + numSet * 4 > lFeat) return false;
        for (int j = 0; j < numSet; j++)
        {
            uint16 val = read16(pSet);
            if (val > maxVal) maxVal = val;
            if (j == 0) defVals[i] = val;
            uint16 label = read16(pSet);
#ifndef DISABLE_TRACING
            XmlTraceLog::get().openElement(ElementFeatureSetting);
            XmlTraceLog::get().addAttribute(AttrIndex, j);
            XmlTraceLog::get().addAttribute(AttrValue, val);
            XmlTraceLog::get().addAttribute(AttrLabel, label);
            if (j == 0) XmlTraceLog::get().addAttribute(AttrDefault, defVals[i]);
            XmlTraceLog::get().closeElement(ElementFeatureSetting);
#endif
        }
        uint32 mask = 1;
        byte bits = 0;
        for (bits = 0; bits < 32; bits++, mask <<= 1)
        {
            if (mask > maxVal)
            {
                if (bits + currBits > 32)
                {
                    currIndex++;
                    currBits = 0;
		    mask = 1;
                }
                currBits += bits;
                m_feats[i] = FeatureRef(currBits, currIndex, (mask - 1) << currBits);
                break;
            }
        }
#ifndef DISABLE_TRACING
    XmlTraceLog::get().closeElement(ElementFeature);
#endif
    }
    m_defaultFeatures = new Features(currIndex + 1);
    for (int i = 0; i < m_numFeats; i++)
        m_defaultFeatures->addFeature(m_feats + i, defVals[i]);

#ifndef DISABLE_TRACING
    XmlTraceLog::get().closeElement(ElementFeatures);
#endif

    return true;
}

bool FeatureMap::readSill(const IFace *face)
{
    size_t lSill;
    char *pSill = (char *)(face->getTable(ktiSill, &lSill));
    char *pBase = pSill;

    if (!pSill) return true;
    if (lSill < 12) return false;
    if (read32(pSill) != 0x00010000) return false;
    m_numLanguages = read16(pSill);
    m_langFeats = new LangFeaturePair[m_numLanguages];
    if (!m_langFeats) { m_numLanguages = 0; return NULL; }		//defensive

    pSill += 6;     // skip the fast search
    if (lSill < m_numLanguages * 8U + 12) return false;

    for (int i = 0; i < m_numLanguages; i++)
    {
        uint32 langid = read32(pSill);
        uint16 numSettings = read16(pSill);
        uint16 offset = read16(pSill);
        if (offset + 8U * numSettings > lSill && numSettings > 0) return false;
        FeaturesHandle feats = cloneFeatures(0/*0 means default*/);
        char *pLSet = pBase + offset;

        for (int j = 0; j < numSettings; j++)
        {
            uint32 name = read32(pLSet);
            uint16 val = read16(pLSet);
            pLSet += 2;
            feats->addFeature(featureRef(name), val);
        }
        //std::pair<uint32, Features *>kvalue = std::pair<uint32, Features *>(langid, feats);
        //m_langMap.insert(kvalue);
        m_langFeats[i].m_lang = langid;
        m_langFeats[i].m_pFeatures = feats;
    }
    return true;
}

FeatureRef *FeatureMap::featureRef(uint32 name)
{
    // TODO reimplement without MAP (nothing is currently put int the map anyway!)
//    std::map<uint32, byte>::iterator res = m_map.find(name);
//    return res == m_map.end() ? NULL : m_feats + res->second;
    return NULL;
}

FeaturesHandle FeatureMap::cloneFeatures(uint32 langname/*0 means default*/) const
{
    if (langname)
    {
        // the number of languages in a font is usually small e.g. 8 in Doulos
        // so this loop is not very expensive
        for (uint16 i = 0; i < m_numLanguages; i++)
        {
            if (m_langFeats[i].m_lang == langname)
                return m_langFeats[i].m_pFeatures->clone();
        }
//        std::map<uint32, Features *>::const_iterator res = m_langMap.find(name);
//        if (res != m_langMap.end()) 
//            return new Features(*res->second);
    }
    return m_defaultFeatures->clone();
}

