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
#include "Main.h"
#include "Pass.h"
#include <string.h>
#include <assert.h>
#include "GrSegment.h"
#include "Code.h"
#include "XmlTraceLog.h"

using vm::Code;
using vm::Machine;
using namespace org::sil::graphite::v2;

Pass::Pass()
 :
    m_silf(NULL),
    m_cols(NULL),
    m_ruleidx(NULL),
    m_ruleMap(NULL),
    m_ruleSorts(NULL),
    m_startStates(NULL),
    m_rulePreCtxt(NULL),
    m_cConstraint(NULL),
    m_cActions(NULL),
    m_sTable(NULL)
{
}

Pass::~Pass()
{
    if (m_cols) free(m_cols);
    if (m_ruleidx) free(m_ruleidx);
    if (m_ruleMap) free(m_ruleMap);
    if (m_startStates) free(m_startStates);
    if (m_ruleSorts) free(m_ruleSorts);
    if (m_rulePreCtxt) free(m_rulePreCtxt);
    if (m_sTable) free(m_sTable);
    delete [] m_cConstraint;
    delete [] m_cActions;
    m_cols = NULL;
    m_ruleidx = NULL;
    m_ruleMap = NULL;
    m_startStates = NULL;
    m_ruleSorts = NULL;
    m_rulePreCtxt = NULL;
    m_sTable = NULL;
    m_cConstraint = NULL;
    m_cActions = NULL;
}

bool Pass::readPass(void *pPass, size_t lPass)
{
    byte *p = (byte *)pPass;
    byte *ePass = (byte *)pPass + lPass;
    uint16 numRanges, numEntries;
#ifndef DISABLE_TRACING
    XmlTraceLog::get().addAttribute(AttrFlags, *p);
#endif
    if (lPass < 40) return false;
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
    m_cols = gralloc<uint16>(m_numGlyphs);
    for (int i = 0; i < m_numGlyphs; i++)
        m_cols[i] = -1;
    if (p + numRanges * 6 >= ePass) return false;
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
    if (p + (m_sSuccess + 1) * sizeof(uint16) >= ePass) return false;
    m_ruleidx = gralloc<uint16>(m_sSuccess+1);
    for (int i = 0; i <= m_sSuccess; i++)
    {
        m_ruleidx[i] = read16(p);
        if (m_ruleidx[i] > lPass) return false;
    }
    numEntries = m_ruleidx[m_sSuccess];
    if (p + (numEntries + 2) * sizeof(uint16) >= ePass) return false;
    m_ruleMap = gralloc<uint16>(numEntries);
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
    m_minPreCtxt = *p++;
    m_maxPreCtxt = *p++;
    m_startStates = gralloc<uint16>(m_maxPreCtxt - m_minPreCtxt + 1);
    if (p + (m_maxPreCtxt - m_minPreCtxt + 1) * sizeof(uint16) >= ePass) return false;
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

    if (p + m_numRules * (sizeof(uint16) + sizeof(byte)) >= ePass) return false;
    m_ruleSorts = gralloc<uint16>(m_numRules);
    for (int i = 0; i < m_numRules; ++i)
        m_ruleSorts[i] = read16(p);
    m_rulePreCtxt = gralloc<byte>(m_numRules);
    for (int i = 0; i < m_numRules; ++i)
    {
        m_rulePreCtxt[i] = *p++;
        if (m_rulePreCtxt[i] > m_maxPreCtxt) return false;
    }
    if (p + 3 >= ePass) return false;
    p++;
    uint16 nPConstraint = read16(p);
    byte *pConstraint = p;
    byte *pActions = p + (m_numRules + 1) * 2;
    p += (m_numRules + 1) * 4;
    if (p + (m_sTransition * m_sColumns * sizeof(uint16)) >= ePass) return false;
    m_sTable = gralloc<int16>(m_sTransition * m_sColumns);
    for (int i = 0; i < m_sTransition * m_sColumns; i++)
    {
        m_sTable[i] = read16(p);
        if (m_sTable[i] > m_sRows || m_sTable[i] < 0) return false;
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
    if (p + nPConstraint >= ePass) return false;
    vm::CodeContext *cContexts = gralloc<vm::CodeContext>(nMaxContext + 2);
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
#ifndef DISABLE_TRACING
        XmlTraceLog::get().openElement(ElementConstraints);
#endif
    p += readCodePointers(p, pConstraint, m_cConstraint, m_numRules, true, cContexts, ePass - p); 
#ifndef DISABLE_TRACING
        XmlTraceLog::get().closeElement(ElementConstraints);
#endif

    if (p >= ePass) return false;
    m_cActions = new Code[m_numRules];
#ifndef DISABLE_TRACING
    XmlTraceLog::get().openElement(ElementActions);
#endif
    p += readCodePointers(p, pActions, m_cActions, m_numRules, false, cContexts, ePass - p);
#ifndef DISABLE_TRACING
        XmlTraceLog::get().closeElement(ElementActions);
#endif

    assert(size_t(p - (byte *)pPass) <= lPass);
    if (p > ePass) return false;
    free(cContexts);
    // no debug
    return true;
}

int Pass::readCodePointers(byte *pCode, byte *pPointers, vm::Code *pRes, int num, bool isConstraint, vm::CodeContext *cContexts, int size)
{
    uint16 loffset = read16(pPointers);
    int lid = (loffset || !isConstraint) ? 0 : -1;
    if (loffset > size) return size;
    for (int i = 1; i <= num; i++)
    {
        uint16 noffset = read16(pPointers);
        if (noffset > 0)
        {
            if (noffset > size) return size;
            if (lid >= 0)
            {
#ifndef DISABLE_TRACING
                if (XmlTraceLog::get().active())
                {
                    if (isConstraint)
                    {
                        XmlTraceLog::get().openElement(ElementConstraint);
                        XmlTraceLog::get().addAttribute(AttrIndex, lid);
                    }
                    else
                    {
                        XmlTraceLog::get().openElement(ElementRule);
                        XmlTraceLog::get().addAttribute(AttrIndex, lid);
                        XmlTraceLog::get().addAttribute(AttrSortKey, m_ruleSorts[lid]);
                        XmlTraceLog::get().addAttribute(AttrPrecontext, m_rulePreCtxt[lid]);
                    }
                }
#endif
                pRes[lid] = Code(isConstraint, pCode + loffset, pCode + noffset, cContexts);
#ifndef DISABLE_TRACING
                if (isConstraint)
                    XmlTraceLog::get().closeElement(ElementConstraint);
                else
                    XmlTraceLog::get().closeElement(ElementRule);
#endif
            }
            lid = i;
            loffset = noffset;
        }
    }
    return loffset;
}

void Pass::runGraphite(GrSegment *seg, const GrFace *face, VMScratch *vms) const
{
    int flags = 0;
    Slot *first = seg->first();
    if (!testConstraint(&m_cPConstraint, first, 1, 0, 0, seg, 1, &first, flags))
        return;
    // advance may be negative, so it is dangerous to use unsigned for i
    int loopCount = m_iMaxLoop;
    int maxIndex = 0, currCount = 0;
    int count = 0;
    for (Slot *s = seg->first(); s; )
    {
        s = findNDoRule(seg, s, count, face, vms);
        currCount += count;
        if (currCount <= maxIndex)
        {
            if (--loopCount < 0)
            {
                while (++currCount <= maxIndex && s) s = s->next();
                loopCount = m_iMaxLoop;
                maxIndex = currCount + 1;
            }
        }
        else
        {
            loopCount = m_iMaxLoop;
            maxIndex = currCount + 1;
        }
    }
}

Slot *Pass::findNDoRule(GrSegment *seg, Slot *iSlot, int &count, const GrFace *face, VMScratch *vms) const
{
    int state;
    Slot *startSlot = iSlot;
    int iCtxt = 0;
    int lcount = count;
    for (iCtxt = 0; iCtxt < m_maxPreCtxt && iSlot->prev(); iCtxt++, iSlot = iSlot->prev()) {}
    if (iCtxt < m_minPreCtxt)
    {
        count = 1;
        return startSlot->next();
    }
    state = m_startStates[m_maxPreCtxt - iCtxt];
    
    if (vms->flags() || vms->slotMap(0) != iSlot)
    {
        lcount = 0;
        vms->resetRules();
        while (true)
        {
            if (!iSlot)
            {
                iSlot = seg->last();
                break;
            }
            uint16 gid = iSlot->gid();
            if (gid >= m_numGlyphs) break;
            vms->slotMap(lcount, iSlot);
            uint16 iCol = m_cols[gid];
            ++lcount;
    #ifdef ENABLE_DEEP_TRACING
            if (state >= m_sTransition)
            {
                XmlTraceLog::get().error("Invalid state %d", state);
            }
            if (iCol >= m_sColumns && iCol != 65535)
            {
                XmlTraceLog::get().error("Invalid column %d ID %d for slot %d",
                    iCol, gid, iSlot);
            }
    #endif
            if (iCol == 65535) break;
            state = m_sTable[state * m_sColumns + iCol];
            if (state >= m_sRows - m_sSuccess)
                for (int i = m_ruleidx[state - m_sRows + m_sSuccess]; i < m_ruleidx[state - m_sRows + m_sSuccess + 1]; ++i) {
                    const uint16 rule = m_ruleMap[i];
                    vms->addRule(rule, m_ruleSorts[rule], lcount - iCtxt);
                }            
            if (!state || state >= m_sTransition) break;
            iSlot = iSlot->next();
        }
        vms->slotMap(lcount, iSlot ? iSlot->next() : iSlot);
        count = lcount;
    }
    
    for (int i = 0; i < vms->ruleLength(); i++)
    {
        int flags = 0;
        int rulenum = vms->rule(i);
#ifdef ENABLE_DEEP_TRACING
        if (XmlTraceLog::get().active())
        {
	        XmlTraceLog::get().openElement(ElementTestRule);
	        XmlTraceLog::get().addAttribute(AttrNum, rulenum);
	        XmlTraceLog::get().addAttribute(AttrIndex, count);
        }
#endif
        if (testConstraint(m_cConstraint + rulenum, startSlot, vms->length(i), m_rulePreCtxt[rulenum], iCtxt, seg, lcount, vms->map(), flags))
        {
#ifdef ENABLE_DEEP_TRACING
            if (XmlTraceLog::get().active())
            {
	          XmlTraceLog::get().closeElement(ElementTestRule);
	          XmlTraceLog::get().openElement(ElementDoRule);
	          XmlTraceLog::get().addAttribute(AttrNum, vms->rule(i));
	          XmlTraceLog::get().addAttribute(AttrIndex, count);
            }
#endif
	    Slot *res = doAction(m_cActions + rulenum, startSlot, count, iCtxt, vms->length(i), seg, vms->map(), flags);
#ifdef ENABLE_DEEP_TRACING
            if (XmlTraceLog::get().active())
            {
    //	      XmlTraceLog::get().addAttribute(AttrResult, res);
            XmlTraceLog::get().closeElement(ElementDoRule);
            }
#endif
//            if (res == -1)
//                return m_ruleSorts[rulenum];
//            else
            vms->setflag(flags);
            return res;
        }
#ifdef ENABLE_DEEP_TRACING
	else
	    XmlTraceLog::get().closeElement(ElementTestRule);
#endif
    }
    count = 1;
    return startSlot->next();
}

int Pass::testConstraint(const Code *codeptr, Slot *iSlot, int num, int nPre, int nCtxt, GrSegment *seg, int nMap, Slot **map, int &flags) const
{
    uint32 ret = 1;
    int count;
    
    if (!*codeptr)
        return 1;
 
    assert(codeptr->constraint());
    
    Machine::status_t status = Machine::finished;
    Machine m;
    for (int i = 0; i < nPre; ++i)
    {
        if (!iSlot->prev())
            break;
        else
            iSlot = iSlot->prev();
//        num++;
    }

    for (int i = -nPre; i < num; i++, iSlot = iSlot->next())
    {
        int temp = i + nCtxt;
        ret = codeptr->run(m, *seg, iSlot, temp, nCtxt, nMap, map, status, flags);
        if (!ret) return 0;
        if (status != Machine::finished) return 1;
    }
    
    return status == Machine::finished ? ret : 1;
}

Slot *Pass::doAction(const Code *codeptr, Slot *iSlot, int &count, int nPre, int len, GrSegment *seg, Slot **map, int &flags) const
{
    if (!*codeptr)
        return iSlot->next();
    
    assert(!codeptr->constraint());
    
    Machine::status_t status;
    Machine m;
    int nMap = count;
    count = nPre;
    int oldNumGlyphs = seg->length();
    int32 ret = codeptr->run(m, *seg, iSlot, count, nPre, nMap, map, status, flags);
    count += seg->length() - oldNumGlyphs;
    
    for (int i = 0; i < nMap; ++i)
    {
        if (map[i]->isCopied() || map[i]->isDeleted())
            seg->freeSlot(map[i]);
    }
    if (ret < 0)
    {
        if (!iSlot)
        {
            iSlot = seg->last();
            ++ret;
            --count;
        }
        while (++ret <= 0)
        {
            iSlot = iSlot->prev();
            --count;
        }
    }
    else if (ret > 0)
    {
        if (!iSlot)
        {
            iSlot = seg->first();
            --ret;
            ++count;
        }
        while (--ret >= 0)
        {
            iSlot = iSlot->next();
            ++count;
        }
    }
    if (status != Machine::finished) return iSlot->next();
    count -= nPre;
    return iSlot;
}

