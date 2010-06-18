#include <string.h>

#include "FeatureMap.h"
#include "Features.h"
#include "graphiteng/IFace.h"
#include "Main.h"

#include "TtfUtil.h"

#define ktiFeat MAKE_TAG('F','e','a','t')

bool FeatureMap::readFont(IFace *face)
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
    if (m_numFeats * 16 + 12 > lFeat) return false;
    m_feats = new FeatureRef[m_numFeats];
    defVals = new uint16[m_numFeats];
    byte currIndex = 0;
    byte currBits = 0;

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

        if (offset + numSet * 4 > lFeat) return false;
        for (int j = 0; j < numSet; j++)
        {
            uint16 val = read16(pSet);
            if (val > maxVal) maxVal = val;
            if (j == 0) defVals[i] = val;
            read16(pSet);
        }
        uint32 mask = 1;
        byte bits = 0;
        for (bits = 0; bits < 32; bits++)
        {
            if (mask > maxVal)
            {
                if (bits + currBits > 32)
                {
                    currIndex++;
                    currBits = 0;
                }
                currBits += bits - 1;
                m_feats[i].init(currBits, currIndex, mask - 1);
                break;
            }
        }
    }

    m_defaultFeatures = new(currIndex + 1) Features(currIndex + 1);
    for (int i = 0; i < m_numFeats; i++)
        m_defaultFeatures->addFeature(m_feats[i], defVals[i]);
    return true;
}

FeatureRef *FeatureMap::featureRef(uint32 name)
{
    std::map<uint32, byte>::iterator res = m_map.find(name);
    return res == m_map.end() ? NULL : m_feats + res->second;
}

IFeatures *FeatureMap::newFeatures()
{
    return m_defaultFeatures->newCopy();
}

