#include "Silf.h"

bool Silf::readGraphite(void *pSilf, int numGlyphs, uint32 version)
{
    char *p = (char *)pSilf;
    uint32 *pPasses;

    if (version >= 0x00030000)
        p += 8;
    p += 2;     // maxGlyphID
    p += 4;     // extra ascent/descent
    m_numPasses = *p++;
    m_passes = new Pass[m_numPasses];
    m_sPass = *p++;
    m_pPass = *p++;
    m_jPass = *p++;
    m_bPass = *p++;     // when do we reorder?
    m_flags = *p++;
    p += 2;     // ignore line end contextuals for now
    p++;        // not sure what to do with attrPseudo
    m_aBreak = *p++;
    p++;        // we don't do bidi
    p += 2;     // skip reserved stuff
    p += *p * 8 + 1;     // ignore justification for now
    m_aLig = read16(p);
    m_aUser = *p++;
    m_iMaxComp = *p++;
    p += 5;     // skip direction and reserved
    p += *p * 2 + 1;        // don't need critical features yet
    p++;        // reserved
    p += *p * 4 + 1;        // skip scripts
    p += 2;     // skip lbGID
    pPasses = (uint32 *)p;
    p += 4 * (m_numPasses + 1);
    uint16 numPseudo = read16(p);
    p += numPseudo * 6 + 6;  // skip pseudo maps

    p += readClassMap((void *)p, swap32(*pPasses) - (p - (char *)pSilf));

    for (int i = 0; i < m_numPasses; i++)
    {
        if (!m_passes[i].readPass((char *)pSilf + swap32(pPasses[i]), swap32(pPasses[i + 1]) - swap32(pPasses[i]), numGlyphs + numPseudo)) return false;
    }
}

size_t Silf::readClassMap(void *pClass, size_t lClass)
{
    uint16 *p = (uint16 *)pClass;
    uint16 nClass, nLinear;
    nClass = swap16(*p); p++;
    nLinear = swap16(*p); p++;
    
    // quick find the length
    p += nClass;
    return swap16(*p);
}

