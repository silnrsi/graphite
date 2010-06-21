#include "Main.h"
#include "Pass.h"
#include <string.h>
#include <assert.h>
#include "Segment.h"

bool Pass::readPass(void *pPass, size_t lPass, int numGlyphs)
{
    byte *p = (byte *)pPass;
    uint16 numRanges, numEntries;

    p++;
    m_iMaxLoop = *p++;
    p += 2;         // don't care about context
    m_numRules = read16(p);
    p += 2;         // not sure why we would want this
    p += 16;         // ignore offsets for now
    m_sRows = read16(p);
    m_sTransition = read16(p);
    if (m_sTransition > m_sRows) return false;
    m_sSuccess = read16(p);
    if (m_sSuccess > m_sRows) return false;
    m_sColumns = read16(p);
    numRanges = read16(p);
    p += 6;
    m_cols = new uint16[numGlyphs];
    for (int i = 0; i < numRanges; i++)
    {
        uint16 first = read16(p);
        uint16 last = read16(p);
        if (last >= numGlyphs) last = numGlyphs - 1;
        uint16 col = read16(p);
        while (first <= last)
            m_cols[first++] = col;
    }
    if (p - (byte *)pPass >= lPass) return false;
    m_ruleidx = new uint16[m_sSuccess + 1];
    for (int i = 0; i <= m_sSuccess; i++)
    {
        m_ruleidx[i] = read16(p);
        if (m_ruleidx[i] < 0 || m_ruleidx[i] > lPass) return false;
    }
    numEntries = m_ruleidx[m_sSuccess];
    m_ruleMap = new uint16[numEntries];
    for (int i = 0; i < numEntries; i++)
    {
        m_ruleMap[i] = read16(p);
    }
    if (p - (byte *)pPass >= lPass) return false;
    m_minPreCtxt = *p++;
    m_maxPreCtxt = *p++;
    m_startStates = new uint16[m_maxPreCtxt - m_minPreCtxt + 1];
    for (int i = 0; i <= m_maxPreCtxt - m_minPreCtxt; i++)
    {
        m_startStates[i] = read16(p);
        if (m_startStates[i] < 0 || m_startStates[i] >= lPass) return false;
    }

    m_ruleSorts = new uint16[m_numRules];
    for (int i = 0; i < m_numRules; i++)
        m_ruleSorts[i] = read16(p);
    m_rulePreCtxt = new byte[m_numRules];
    memcpy(m_rulePreCtxt, p, m_numRules);
    p += m_numRules;
    p++;
    uint16 m_nPConstraint = read16(p);
    m_pConstraint = new uint16[m_numRules + 1];
    m_pActions = new uint16[m_numRules + 1];
    for (int i = 0; i <= m_numRules; i++)
        m_pConstraint[i] = read16(p);
    for (int i = 0; i <= m_numRules; i++)
        m_pActions[i] = read16(p);
    m_sTable = new int16[m_sTransition * m_sColumns];
    for (int i = 0; i < m_sTransition * m_sColumns; i++)
        m_sTable[i] = read16(p);
    p++;
    if (m_nPConstraint)
    {
        m_cPConstraint = new byte[m_nPConstraint];
        memcpy(m_cPConstraint, p, m_nPConstraint);
        p += m_nPConstraint;
    }
    m_cConstraint = new byte[m_pConstraint[m_numRules]];
    memcpy(m_cConstraint, p, m_pConstraint[m_numRules]);
    p += m_pConstraint[m_numRules];
    m_cActions = new byte[m_pActions[m_numRules]];
    memcpy(m_cActions, p, m_pActions[m_numRules]);
    p += m_pActions[m_numRules];

    assert(p - (byte *)pPass <= lPass);
    // no debug
    return true;
}

void Pass::runGraphite(Segment *seg, FontFace *face, Silf *silf, VMScratch *vms)
{
    if (m_nPConstraint && !testConstraint(m_cPConstraint, m_nPConstraint, 0, seg, silf, vms))
        return;

    for (int i = 0; i < seg->length(); i++)
    {
        int advance = 0;
        int loopCount = m_iMaxLoop;
        while (!advance && loopCount--)
            advance = findNDoRule(seg, i, vms, silf);
        if (advance > 0) i += advance - 1;
    }
}

int Pass::findNDoRule(Segment *seg, int iSlot, VMScratch *vms, Silf *silf)
{
    int state;
    int startSlot = iSlot;
    if (iSlot < m_minPreCtxt) return -1;
    if (iSlot < m_maxPreCtxt)
    {
        state = m_startStates[m_maxPreCtxt - iSlot];
        iSlot = 0;
    }
    else
    {
        state = 0;
        iSlot -= m_maxPreCtxt;
    }

    vms->resetRules();
    while (true)
    {
        if (iSlot >= seg->length()) break;
        state = m_sTable[state * m_sColumns + m_cols[(*seg)[iSlot].gid()]];
        if (state > m_sSuccess)
            for (int i = m_ruleidx[state - m_sSuccess]; i < m_ruleidx[state - m_sSuccess + 1]; i++)
                vms->addRule(m_ruleMap[i], m_ruleSorts[i]);
        if (!state || state >= m_sTransition) break;
        iSlot++;
    }
    
    for (int i = 0; i < vms->ruleLength(); i++)
    {
        int offset = m_pConstraint[vms->rule(i)];
        if (testConstraint(m_cConstraint + offset, m_pConstraint[vms->rule(i) + 1] - offset, startSlot, seg, silf, vms))
        {
            offset = m_pActions[vms->rule(i)];
            int res = doAction(m_cActions + offset, m_pActions[vms->rule(i) + 1] - offset, startSlot, seg, silf, vms);
            if (res == -1)
                return m_ruleSorts[vms->rule(i)];
            else
                return res;
        }
    }
    return -1;
}

int Pass::testConstraint(byte *code, size_t len, int iSlot, Segment *seg, Silf *silf, VMScratch *vms)
{
    return 1;
}

int Pass::doAction(byte *code, size_t len, int iSlot, Segment *seg, Silf *silf, VMScratch *vms)
{
    return -1;
}


