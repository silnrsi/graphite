#include "Silf.h"
#include "XmlTraceLog.h"

bool Silf::readGraphite(void *pSilf, size_t lSilf, int numGlyphs, uint32 version)
{
    char *p = (char *)pSilf;
    uint32 *pPasses;
    XmlTraceLog::get().openElement(ElementSilfSub);
    if (version >= 0x00030000)
    {
        XmlTraceLog::get().addAttribute(AttrMajor, swap16(((uint16*) p)[0]));
        XmlTraceLog::get().addAttribute(AttrMinor, swap16(((uint16*) p)[1]));
        p += 8;
    }
    p += 2;     // maxGlyphID
    p += 4;     // extra ascent/descent
    m_numPasses = *p++;
    XmlTraceLog::get().addAttribute(AttrNumPasses, m_numPasses);
    if (m_numPasses < 0) return false;
    m_passes = new Pass[m_numPasses];
    m_sPass = *p++;
    XmlTraceLog::get().addAttribute(AttrSubPass, m_sPass);
    if (m_sPass < 0) return false;
    m_pPass = *p++;
    XmlTraceLog::get().addAttribute(AttrPosPass, m_pPass);
    if (m_pPass < m_sPass) return false;
    m_jPass = *p++;
    XmlTraceLog::get().addAttribute(AttrJustPass, m_jPass);
    if (m_jPass < m_pPass) return false;
    m_bPass = *p++;     // when do we reorder?
    XmlTraceLog::get().addAttribute(AttrBidiPass, m_bPass);
    if (m_bPass != 0xFF && (m_bPass < m_jPass || m_bPass > m_numPasses)) return false;
    m_flags = *p++;
    p += 2;     // ignore line end contextuals for now
    p++;        // not sure what to do with attrPseudo
    m_aBreak = *p++;
    XmlTraceLog::get().addAttribute(AttrBreakWeight, m_aBreak);
    if (m_aBreak < 0) return false;
    XmlTraceLog::get().addAttribute(AttrDirectionality, *p);
    p++;        // we don't do bidi
    p += 2;     // skip reserved stuff
    XmlTraceLog::get().addAttribute(AttrNumJustLevels, *p);
    p += *p * 8 + 1;     // ignore justification for now
    m_aLig = read16(p);
    XmlTraceLog::get().addAttribute(AttrLigComp, *p);
    if (m_aLig > 127) return false;
    m_aUser = *p++;
    XmlTraceLog::get().addAttribute(AttrUserDefn, m_aUser);
    if (m_aUser < 0) return false;
    m_iMaxComp = *p++;
    XmlTraceLog::get().addAttribute(AttrNumLigComp, m_iMaxComp);
    if (m_iMaxComp < 0) return false;
    p += 5;     // skip direction and reserved
    XmlTraceLog::get().addAttribute(AttrNumCritFeatures, *p);
    p += *p * 2 + 1;        // don't need critical features yet
    p++;        // reserved
    if (p - (char *)pSilf >= lSilf) return false;
    p += *p * 4 + 1;        // skip scripts
    p += 2;     // skip lbGID
    if (p - (char *)pSilf >= lSilf) return false;
    pPasses = (uint32 *)p;
    p += 4 * (m_numPasses + 1);
    m_numPseudo = read16(p);
    XmlTraceLog::get().addAttribute(AttrNumPseudo, m_numPseudo);
    p += 6;
    m_pseudos = new Pseudo[m_numPseudo];
    for (int i = 0; i < m_numPseudo; i++)
    {
        m_pseudos[i].uid = read32(p);
        m_pseudos[i].gid = read16(p);
    }
    if (p - (char *)pSilf >= lSilf) return false;

    int clen = readClassMap((void *)p, swap32(*pPasses) - (p - (char *)pSilf));
    if (clen < 0) return false;
    p += clen;

    for (int i = 0; i < m_numPasses; i++)
    {
        m_passes[i].init(this);
        XmlTraceLog::get().openElement(ElementPass);
        XmlTraceLog::get().addAttribute(AttrPassId, i);
        if (!m_passes[i].readPass((char *)pSilf + swap32(pPasses[i]), swap32(pPasses[i + 1]) - swap32(pPasses[i]), numGlyphs + m_numPseudo))
        {
            XmlTraceLog::get().closeElement(ElementPass);
            return false;
        }
        XmlTraceLog::get().closeElement(ElementPass);
    }
    XmlTraceLog::get().closeElement(ElementSilfSub);
    return true;
}

size_t Silf::readClassMap(void *pClass, size_t lClass)
{
    char *p = (char *)pClass;

    m_nClass = read16(p);
    m_nLinear = read16(p);
    m_classOffsets = new uint16[m_nClass + 1];

    for (int i = 0; i <= m_nClass; i++)
        m_classOffsets[i] = read16(p) / 2 - (2 + m_nClass);     // uint16[] index
 
    if (m_classOffsets[m_nClass] + (2 + m_nClass) * 2 > lClass) return -1;
    m_classData = new uint16[m_classOffsets[m_nClass]];
    for (int i = 0; i < m_classOffsets[m_nClass]; i++)
        m_classData[i] = read16(p);
    return (p - (char *)pClass);
}

uint16 Silf::findPseudo(uint32 uid)
{
    for (int i = 0; i < m_numPseudo; i++)
        if (m_pseudos[i].uid == uid) return m_pseudos[i].gid;
    return 0;
}

uint16 Silf::findClassIndex(uint16 cid, uint16 gid)
{
    if (cid > m_nClass || cid < 0) return -1;

    uint16 loc = m_classOffsets[cid];
    if (cid < m_nLinear)        // output class being used for input, shouldn't happen
    {
        for (int i = loc; i < m_classOffsets[cid + 1]; i++)
            if (m_classData[i] == gid) return i - loc;
    }
    else
    {
        uint16 num = m_classData[loc];
        uint16 search = m_classData[loc + 1] / 2;
        uint16 selector = m_classData[loc + 2];
        uint16 range = m_classData[loc + 3] / 2;

        uint16 curr = loc + 4 + range;

        while (search > 1)
        {
            int test;
            if (curr < loc + 4)
                test = -1;
            else
                test = m_classData[curr] - gid;

            if (test == 0) return m_classData[curr + 1];

            search >>= 1;
            if (test < 0)
                curr += search;
            else
                curr -= search;
        }
    }
    return -1;
}

uint16 Silf::getClassGlyph(uint16 cid, uint16 index)
{
    if (cid > m_nClass || cid < 0) return 0;

    uint16 loc = m_classOffsets[cid];
    if (cid < m_nLinear)
    {
        if (index < m_classOffsets[cid + 1] - index)
            return m_classData[index + loc];
    }
    else        // input class being used for output. Shouldn't happen
    {
        for (int i = loc + 4; i < m_classOffsets[cid + 1]; i += 2)
            if (m_classData[i + 1] == index) return m_classData[i];
    }
    return 0;
}


void Silf::runGraphite(Segment *seg, FontFace *face, VMScratch *vms)
{
    for (int i = 0; i < m_numPasses; i++)
    {
        // test whether to reorder, prepare for positioning
        m_passes[i].runGraphite(seg, face, this, vms);
    }
}

