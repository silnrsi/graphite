#include "Main.h"
#include "Pass.h"
#include <string.h>
#include <assert.h>
#include "Segment.h"
#include "code.h"

bool Pass::readPass(void *pPass, size_t lPass, int numGlyphs)
{
    byte *p = (byte *)pPass;
    uint16 numRanges, numEntries;
    XmlTraceLog::get().addAttribute(AttrFlags, *p);
    p++;
    m_iMaxLoop = *p++;
    XmlTraceLog::get().addAttribute(AttrMaxRuleLoop, m_iMaxLoop);
    byte nMaxContext = *p++;
    p++;             // don't care about context
    m_numRules = read16(p);
    XmlTraceLog::get().addAttribute(AttrNumRules, m_numRules);
    p += 2;          // not sure why we would want this
    p += 16;         // ignore offsets for now
    m_sRows = read16(p);
    XmlTraceLog::get().addAttribute(AttrNumRows, m_sRows);
    m_sTransition = read16(p);
    XmlTraceLog::get().addAttribute(AttrNumTransition, m_sTransition);
    if (m_sTransition > m_sRows) return false;
    m_sSuccess = read16(p);
    XmlTraceLog::get().addAttribute(AttrNumSuccess, m_sSuccess);
    if (m_sSuccess > m_sRows) return false;
    m_sColumns = read16(p);
    XmlTraceLog::get().addAttribute(AttrNumColumns, m_sColumns);
    numRanges = read16(p);
    XmlTraceLog::get().addAttribute(AttrNumRanges, numRanges);
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
    if (size_t(p - (byte *)pPass) >= lPass) return false;
    m_ruleidx = new uint16[m_sSuccess + 1];
    for (int i = 0; i <= m_sSuccess; i++)
    {
        m_ruleidx[i] = read16(p);
        if (m_ruleidx[i] > lPass) return false;
    }
    numEntries = m_ruleidx[m_sSuccess];
    m_ruleMap = new uint16[numEntries];
    for (int i = 0; i < numEntries; i++)
    {
        m_ruleMap[i] = read16(p);
    }
    if (size_t(p - (byte *)pPass) >= lPass) return false;
    m_minPreCtxt = *p++;
    XmlTraceLog::get().addAttribute(AttrMinPrecontext, m_minPreCtxt);
    m_maxPreCtxt = *p++;
    XmlTraceLog::get().addAttribute(AttrMaxPrecontext, m_maxPreCtxt);
    m_startStates = new uint16[m_maxPreCtxt - m_minPreCtxt + 1];
    for (int i = 0; i <= m_maxPreCtxt - m_minPreCtxt; i++)
    {
        m_startStates[i] = read16(p);
        if (m_startStates[i] >= lPass) return false;
    }

    m_ruleSorts = new uint16[m_numRules];
    for (int i = 0; i < m_numRules; i++)
        m_ruleSorts[i] = read16(p);
    m_rulePreCtxt = new byte[m_numRules];
    memcpy(m_rulePreCtxt, p, m_numRules);
    p += m_numRules;
    p++;
    uint16 nPConstraint = read16(p);
    byte *pConstraint = p;
    byte *pActions = p + (m_numRules + 1) * 2;
    p += (m_numRules + 1) * 4;
    m_sTable = new int16[m_sTransition * m_sColumns];
    for (int i = 0; i < m_sTransition * m_sColumns; i++)
        m_sTable[i] = read16(p);
    p++;
    byte *cContexts = new byte[2 * nMaxContext];
    if (nPConstraint)
    {
        m_cPConstraint = code(true, p, p + nPConstraint, cContexts);
        p += nPConstraint;
    }
    else
        m_cPConstraint = code();

    m_cConstraint = new code[m_numRules];
    uint16 loffset = read16(pConstraint);
    for (int i = 0; i < m_numRules; i++)
    {
        uint16 noffset = read16(pConstraint);
        if (noffset > loffset) m_cConstraint[i] = code(true, p + loffset, p + noffset, cContexts);
        loffset = noffset;
    }
    p += loffset;
    m_cActions = new code[m_numRules];
    loffset = read16(pActions);
    for (int i = 0; i < m_numRules; i++)
    {
        uint16 noffset = read16(pActions);
        if (noffset > loffset) m_cActions[i] = code(false, p + loffset, p + noffset, cContexts);
        loffset = noffset;
    }
    p += loffset;

    assert(size_t(p - (byte *)pPass) <= lPass);
    // no debug
    return true;
}

void Pass::runGraphite(Segment *seg, FontFace *face, Silf *silf, VMScratch *vms)
{
    if (!testConstraint(&m_cPConstraint, 0, seg, silf, vms))
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
        if (testConstraint(m_cConstraint + vms->rule(i), startSlot, seg, silf, vms))
        {
            int res = doAction(m_cActions + vms->rule(i), startSlot, seg, silf, vms);
            if (res == -1)
                return m_ruleSorts[vms->rule(i)];
            else
                return res;
        }
    }
    return -1;
}

int Pass::testConstraint(code *code, int iSlot, Segment *seg, Silf *silf, VMScratch *vms)
{
    if (!*code)
        return 1;
    
    machine::status_t status;
    const uint32 ret = code->run(vms->stack(), size_t(64), *seg, iSlot, status);
    
    return status == machine::finished ? ret : 1;
}

int Pass::doAction(code *code, int iSlot, Segment *seg, Silf *silf, VMScratch *vms)
{
    if (!*code)
        return 1;
    
    machine::status_t status;
    int iStart = iSlot;
    const uint32 ret = code->run(vms->stack(), size_t(64), *seg, iSlot, status);
    
    while (iStart < iSlot)
    {
        if ((*seg)[iStart].isDeleted())
        {
            seg->deleteSlot(iStart);
            iSlot--;
        }
        else
            iStart++;
    }
    return status == machine::finished ? iSlot + ret : iSlot ;
}


