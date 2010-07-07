#include "Main.h"
#include "Pass.h"
#include <string.h>
#include <assert.h>
#include "Segment.h"
#include "Code.h"
#include "XmlTraceLog.h"

using vm::Code;
using vm::Machine;
using namespace org::sil::graphite::v2;

bool Pass::readPass(void *pPass, size_t lPass)
{
    byte *p = (byte *)pPass;
    uint16 numRanges, numEntries;
#ifndef DISABLE_TRACING
    XmlTraceLog::get().addAttribute(AttrFlags, *p);
#endif
    p++;
    m_iMaxLoop = *p++;
#ifndef DISABLE_TRACING
    XmlTraceLog::get().addAttribute(AttrMaxRuleLoop, m_iMaxLoop);
#endif
    byte nMaxContext = *p++;
    p++;             // don't care about context
    m_numRules = read16(p);
#ifndef DISABLE_TRACING
    XmlTraceLog::get().addAttribute(AttrNumRules, m_numRules);
#endif
    p += 2;          // not sure why we would want this
    p += 16;         // ignore offsets for now
    m_sRows = read16(p);
#ifndef DISABLE_TRACING
    XmlTraceLog::get().addAttribute(AttrNumRows, m_sRows);
#endif
    m_sTransition = read16(p);
#ifndef DISABLE_TRACING
    XmlTraceLog::get().addAttribute(AttrNumTransition, m_sTransition);
#endif
    if (m_sTransition > m_sRows) return false;
    m_sSuccess = read16(p);
#ifndef DISABLE_TRACING
    XmlTraceLog::get().addAttribute(AttrNumSuccess, m_sSuccess);
#endif
    if (m_sSuccess > m_sRows) return false;
    m_sColumns = read16(p);
#ifndef DISABLE_TRACING
    XmlTraceLog::get().addAttribute(AttrNumColumns, m_sColumns);
#endif
    numRanges = read16(p);
#ifndef DISABLE_TRACING
    XmlTraceLog::get().addAttribute(AttrNumRanges, numRanges);
#endif
    p += 6;
    m_numGlyphs = swap16(*(uint16 *)(p + numRanges * 6 - 4)) + 1;
    m_cols = new uint16[m_numGlyphs];
    for (int i = 0; i < m_numGlyphs; i++)
        m_cols[i] = -1;
    for (int i = 0; i < numRanges; i++)
    {
        uint16 first = read16(p);
        uint16 last = read16(p);
        uint16 col = read16(p);
#ifndef DISABLE_TRACING
        if (XmlTraceLog::get().active())
        {
            XmlTraceLog::get().openElement(ElementRange);
            XmlTraceLog::get().addAttribute(AttrFirstId, first);
            XmlTraceLog::get().addAttribute(AttrLastId, last);
            XmlTraceLog::get().addAttribute(AttrColId, col);
            XmlTraceLog::get().closeElement(ElementRange);
        }
#endif
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
#ifndef DISABLE_TRACING
    if (XmlTraceLog::get().active())
    {
        for (uint16 iSuccess = 0; iSuccess < m_sSuccess; iSuccess++)
        {
            XmlTraceLog::get().openElement(ElementRuleMap);
            XmlTraceLog::get().addAttribute(AttrSuccessId, iSuccess);
            for (uint16 j = m_ruleidx[iSuccess]; j < m_ruleidx[iSuccess+1]; j++)
            {
                XmlTraceLog::get().openElement(ElementRule);
                XmlTraceLog::get().addAttribute(AttrRuleId, m_ruleMap[j]);
                XmlTraceLog::get().closeElement(ElementRule);
            }
            XmlTraceLog::get().closeElement(ElementRuleMap);
        }
    }
#endif
    if (size_t(p - (byte *)pPass) >= lPass) return false;
    m_minPreCtxt = *p++;
    m_maxPreCtxt = *p++;
    m_startStates = new uint16[m_maxPreCtxt - m_minPreCtxt + 1];
    for (int i = 0; i <= m_maxPreCtxt - m_minPreCtxt; i++)
    {
        m_startStates[i] = read16(p);
#ifndef DISABLE_TRACING
        if (XmlTraceLog::get().active())
        {
            XmlTraceLog::get().openElement(ElementStartState);
            XmlTraceLog::get().addAttribute(AttrContextLen, i + m_minPreCtxt);
            XmlTraceLog::get().addAttribute(AttrState, m_startStates[i]);
            XmlTraceLog::get().closeElement(ElementStartState);
        }
#endif
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
    {
        m_sTable[i] = read16(p);
    }
#ifndef DISABLE_TRACING
    if (XmlTraceLog::get().active())
    {
        XmlTraceLog::get().openElement(ElementStateTransitions);
        for (size_t iRow = 0; iRow < m_sTransition; iRow++)
        {
            XmlTraceLog::get().openElement(ElementRow);
            XmlTraceLog::get().addAttribute(AttrIndex, iRow);
            size_t nRowOffset = iRow * m_sColumns;
            XmlTraceLog::get().writeElementArray(ElementData, AttrValue, 
                m_sTable + nRowOffset, m_sColumns);
            XmlTraceLog::get().closeElement(ElementRow);
        }
        XmlTraceLog::get().closeElement(ElementStateTransitions);
    }
#endif
    p++;
    byte *cContexts = new byte[2 * nMaxContext];
    if (nPConstraint)
    {
#ifndef DISABLE_TRACING    
        XmlTraceLog::get().openElement(ElementConstraint);
#endif
        m_cPConstraint = Code(true, p, p + nPConstraint, cContexts);
#ifndef DISABLE_TRACING
        XmlTraceLog::get().closeElement(ElementConstraint);
#endif
        p += nPConstraint;
    }
    else
        m_cPConstraint = Code();

    m_cConstraint = new Code[m_numRules];
    uint16 loffset = read16(pConstraint);
#ifndef DISABLE_TRACING
        XmlTraceLog::get().openElement(ElementConstraints);
#endif
    for (uint16 i = 0; i < m_numRules; i++)
    {
        uint16 noffset = read16(pConstraint);
#ifndef DISABLE_TRACING
        XmlTraceLog::get().openElement(ElementConstraint);
        XmlTraceLog::get().addAttribute(AttrIndex, i);
#endif
        if (noffset > loffset) m_cConstraint[i] = Code(true, p + loffset, p + noffset, cContexts);
#ifndef DISABLE_TRACING
        XmlTraceLog::get().closeElement(ElementConstraint);
#endif
        loffset = noffset;
    }
#ifndef DISABLE_TRACING
        XmlTraceLog::get().closeElement(ElementConstraints);
#endif
    p += loffset;
    m_cActions = new Code[m_numRules];
    loffset = read16(pActions);
#ifndef DISABLE_TRACING
    XmlTraceLog::get().openElement(ElementActions);
#endif
    for (int i = 0; i < m_numRules; i++)
    {
#ifndef DISABLE_TRACING
        if (XmlTraceLog::get().active())
        {
            XmlTraceLog::get().openElement(ElementRule);
            XmlTraceLog::get().addAttribute(AttrIndex, i);
            XmlTraceLog::get().addAttribute(AttrSortKey, m_ruleSorts[i]);
            XmlTraceLog::get().addAttribute(AttrPrecontext, m_rulePreCtxt[i]);
        }
#endif
        uint16 noffset = read16(pActions);
        if (noffset > loffset) m_cActions[i] = Code(false, p + loffset, p + noffset, cContexts);
        loffset = noffset;
#ifndef DISABLE_TRACING
        XmlTraceLog::get().closeElement(ElementRule);
#endif
    }
#ifndef DISABLE_TRACING
        XmlTraceLog::get().closeElement(ElementActions);
#endif
    p += loffset;

    assert(size_t(p - (byte *)pPass) <= lPass);
    // no debug
    return true;
}

void Pass::runGraphite(Segment *seg, const GrFace *face, VMScratch *vms) const
{
    if (!testConstraint(&m_cPConstraint, 0, 1, seg, vms))
        return;

    for (unsigned int i = 0; i < seg->length(); i++)
    {
        int advance = 0;
        int loopCount = m_iMaxLoop;
        while (!advance && loopCount--)
            advance = findNDoRule(seg, i, face, vms);
        if (advance == 0) advance = 1;
        i += advance - 1;
    }
}

int Pass::findNDoRule(Segment *seg, int iSlot, const GrFace *face, VMScratch *vms) const
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
        if (iSlot >= (int)seg->length()) break;
        uint16 gid = (*seg)[iSlot].gid();
        if (gid >= m_numGlyphs) break;
        uint16 iCol = m_cols[gid];
#ifdef ENABLE_DEEP_TRACING
        if (state >= m_sTransition)
        {
            XmlTraceLog::get().error("Invalid state %d", state);
        }
        if (iCol >= m_sColumns && iCol != 65535)
        {
            XmlTraceLog::get().error("Invalid column %d ID %d for slot %d",
                m_cols[(*seg)[iSlot].gid()], (*seg)[iSlot].gid(), iSlot);
        }
#endif
        if (iCol == 65535) break;
        state = m_sTable[state * m_sColumns + iCol];
        if (state > m_sRows - m_sSuccess)
            for (int i = m_ruleidx[state - m_sRows + m_sSuccess]; i < m_ruleidx[state - m_sRows + m_sSuccess + 1]; ++i) {
                const uint16 rule = m_ruleMap[i];
                vms->addRule(rule, m_ruleSorts[rule], iSlot - startSlot);
            }            
        if (!state || state >= m_sTransition) break;
        iSlot++;
    }
    
    for (int i = 0; i < vms->ruleLength(); i++)
    {
        int rulenum = vms->rule(i);
#ifdef ENABLE_DEEP_TRACING
        if (XmlTraceLog::get().active())
        {
	        XmlTraceLog::get().openElement(ElementTestRule);
	        XmlTraceLog::get().addAttribute(AttrNum, rulenum);
	        XmlTraceLog::get().addAttribute(AttrIndex, startSlot);
        }
#endif
        if (testConstraint(m_cConstraint + rulenum, startSlot, vms->length(i), seg, vms))
        {
#ifdef ENABLE_DEEP_TRACING
            if (XmlTraceLog::get().active())
            {
	          XmlTraceLog::get().closeElement(ElementTestRule);
	          XmlTraceLog::get().openElement(ElementDoRule);
	          XmlTraceLog::get().addAttribute(AttrNum, vms->rule(i));
	          XmlTraceLog::get().addAttribute(AttrIndex, startSlot);
            }
#endif
	  int res = doAction(m_cActions + rulenum, startSlot, seg, vms);
#ifdef ENABLE_DEEP_TRACING
        if (XmlTraceLog::get().active())
        {
//	      XmlTraceLog::get().addAttribute(AttrResult, res);
    	  XmlTraceLog::get().closeElement(ElementDoRule);
        }
#endif
            if (res == -1)
                return m_ruleSorts[rulenum];
            else
                return res;
        }
#ifdef ENABLE_DEEP_TRACING
	else
	    XmlTraceLog::get().closeElement(ElementTestRule);
#endif
    }
    return 1;
}

int Pass::testConstraint(const Code *codeptr, int iSlot, int num, Segment *seg, VMScratch *vms) const
{
    uint32 ret;
    
    if (!*codeptr)
        return 1;
 
    assert(codeptr->constraint());
    
    Machine::status_t status;
    Machine m;
    for (int i = 0; i <= num; i++)
    {
        int is = iSlot + i;
        ret = codeptr->run(m, *seg, is, iSlot, status);
        if (!ret) return 0;
        if (status != Machine::finished) return 1;
    }
    
    return status == Machine::finished ? ret : 1;
}

int Pass::doAction(const Code *codeptr, int iSlot, Segment *seg, VMScratch *vms) const
{
    if (!*codeptr)
        return 1;
    
    assert(!codeptr->constraint());
    
    Machine::status_t status;
    Machine m;
    int iStart = iSlot;
    int32 ret = codeptr->run(m, *seg, iSlot, iSlot, status);
    
    if (iSlot >= int(seg->length()))
    {
        ret += seg->length() - iSlot + 1;
        iSlot = seg->length() - 1;
    }
    for (int i = iStart; i <= iSlot; )
    {
        if ((*seg)[i].isDeleted())
        {
            seg->deleteSlot(i);
            iSlot--;
        }
        else
            i++;
    }
    ret += iSlot;
    return (status == Machine::finished ? ret : iSlot) - iStart;
}

