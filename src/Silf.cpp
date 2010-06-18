#include "Silf.h"

bool Silf::readGraphite(void *pSilf, size_t lSilf, int numGlyphs, uint32 version)
{
    char *p = (char *)pSilf;
    uint32 *pPasses;

    if (version >= 0x00030000)
        p += 8;
    p += 2;     // maxGlyphID
    p += 4;     // extra ascent/descent
    m_numPasses = *p++;
    if (m_numPasses < 0) return false;
    m_passes = new Pass[m_numPasses];
    m_sPass = *p++;
    if (m_sPass < 0) return false;
    m_pPass = *p++;
    if (m_pPass < m_sPass) return false;
    m_jPass = *p++;
    if (m_jPass < m_pPass) return false;
    m_bPass = *p++;     // when do we reorder?
    if (m_bPass != -1 && (m_bPass < m_jPass || m_bPass > m_numPasses)) return false;
    m_flags = *p++;
    p += 2;     // ignore line end contextuals for now
    p++;        // not sure what to do with attrPseudo
    m_aBreak = *p++;
    if (m_aBreak < 0) return false;
    p++;        // we don't do bidi
    p += 2;     // skip reserved stuff
    p += *p * 8 + 1;     // ignore justification for now
    m_aLig = read16(p);
    if (m_aLig > 127) return false;
    m_aUser = *p++;
    if (m_aUser < 0) return false;
    m_iMaxComp = *p++;
    if (m_iMaxComp < 0) return false;
    p += 5;     // skip direction and reserved
    p += *p * 2 + 1;        // don't need critical features yet
    p++;        // reserved
    if (p - (char *)pSilf >= lSilf) return false;
    p += *p * 4 + 1;        // skip scripts
    p += 2;     // skip lbGID
    if (p - (char *)pSilf >= lSilf) return false;
    pPasses = (uint32 *)p;
    p += 4 * (m_numPasses + 1);
    uint16 numPseudo = read16(p);
    p += numPseudo * 6 + 6;  // skip pseudo maps
    if (p - (char *)pSilf >= lSilf) return false;

    int clen = readClassMap((void *)p, swap32(*pPasses) - (p - (char *)pSilf));
    if (clen < 0) return false;
    p += clen;

    for (int i = 0; i < m_numPasses; i++)
    {
        if (!m_passes[i].readPass((char *)pSilf + swap32(pPasses[i]), swap32(pPasses[i + 1]) - swap32(pPasses[i]), numGlyphs + numPseudo)) return false;
    }
    return true;
}

size_t Silf::readClassMap(void *pClass, size_t lClass)
{
    uint16 *p = (uint16 *)pClass;
    uint16 nClass, nLinear;
    nClass = swap16(*p); p++;
    nLinear = swap16(*p); p++;
    
    // quick find the length
    p += nClass;
    if (swap16(*p) < 0 || swap16(*p) > lClass) return -1;
    return swap16(*p);
}

void Silf::runGraphite(Segment *seg, FontFace *face, VMScratch *vms)
{
    for (int i = 0; i < m_numPasses; i++)
    {
        // test whether to reorder, prepare for positioning
        m_passes[i].runGraphite(seg, face, this, vms);
    }
}

