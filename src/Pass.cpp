/*  GRAPHITE2 LICENSING

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
#include <stdlib.h>
#include <assert.h>
#include "Segment.h"
#include "Code.h"
#include "Rule.h"
#include "XmlTraceLog.h"

using namespace graphite2;

using vm::Code;
using vm::Machine;


Pass::Pass()
        :
        m_silf(0),
        m_cols(0),
        m_rules(0),
        m_ruleMap(0),
        m_startStates(0),
        m_sTable(0),
        m_states(0)
{
}

Pass::~Pass()
{
    free(m_cols);
    free(m_startStates);
    free(m_sTable);
    free(m_states);
    free(m_ruleMap);
    free(m_rules);
}

bool Pass::readPass(void *pass, size_t pass_length, size_t subtable_base, const Face & face)
{
    const byte *                p = reinterpret_cast<const byte *>(pass),
               * const pass_start = p,
               * const pass_end   = p + pass_length;
    size_t numRanges;

    // Read in basic values
    m_immutable = (*p++) & 0x1U;
    m_iMaxLoop = *p++;
    *p++; // skip maxContext
    p += sizeof(byte);     // skip maxBackup
    m_numRules = read16(p);
    p += sizeof(uint16);   // not sure why we would want this
    const byte * const pcCode = pass_start + read32(p) - subtable_base,
               * const rcCode = pass_start + read32(p) - subtable_base,
               * const aCode  = pass_start + read32(p) - subtable_base;
    p += sizeof(uint32);
    m_sRows = read16(p);
    m_sTransition = read16(p);
    m_sSuccess = read16(p);
    m_sColumns = read16(p);
    numRanges = read16(p);
    p += sizeof(uint16)   // skip searchRange
         +  sizeof(uint16)   // skip entrySelector
         +  sizeof(uint16);  // skip rangeShift
#ifndef DISABLE_TRACING
    XmlTraceLog::get().addAttribute(AttrFlags,          m_immutable);
    XmlTraceLog::get().addAttribute(AttrMaxRuleLoop,    m_iMaxLoop);
    XmlTraceLog::get().addAttribute(AttrNumRules,       m_numRules);
    XmlTraceLog::get().addAttribute(AttrNumRows,        m_sRows);
    XmlTraceLog::get().addAttribute(AttrNumTransition,  m_sTransition);
    XmlTraceLog::get().addAttribute(AttrNumSuccess,     m_sSuccess);
    XmlTraceLog::get().addAttribute(AttrNumColumns,     m_sColumns);
    XmlTraceLog::get().addAttribute(AttrNumRanges,      numRanges);
#endif
    assert(p - pass_start == 40);
    // Perform some sanity checks.
    if (   m_sTransition > m_sRows
            || m_sSuccess > m_sRows)
        return false;

    m_numGlyphs = swap16(*(uint16 *)(p + numRanges * 6 - 4)) + 1;
    // Caculate the start of vairous arrays.
    const uint16 * const ranges = reinterpret_cast<const uint16 *>(p);
    p += numRanges*sizeof(uint16)*3;
    const uint16 * const o_rule_map = reinterpret_cast<const uint16 *>(p);
    p += (m_sSuccess + 1)*sizeof(uint16);

    // More sanity checks
    if (   reinterpret_cast<const byte *>(o_rule_map) > pass_end
            || p > pass_end)
        return false;
    const size_t numEntries = swap16(o_rule_map[m_sSuccess]);
    const uint16 * const   rule_map = reinterpret_cast<const uint16 *>(p);
    p += numEntries*sizeof(uint16);

    if (p > pass_end) return false;
    m_minPreCtxt = *p++;
    m_maxPreCtxt = *p++;
    const int16 * const start_states = reinterpret_cast<const int16 *>(p);
    p += (m_maxPreCtxt - m_minPreCtxt + 1)*sizeof(int16);
    const uint16 * const sort_keys = reinterpret_cast<const uint16 *>(p);
    p += m_numRules*sizeof(uint16);
    const byte * const precontext = p;
    p += m_numRules;
    p += sizeof(byte);     // skip reserved byte

    if (p > pass_end) return false;
    const size_t pass_constraint_len = read16(p);
    const uint16 * const o_constraint = reinterpret_cast<const uint16 *>(p);
    p += (m_numRules + 1)*sizeof(uint16);
    const uint16 * const o_actions = reinterpret_cast<const uint16 *>(p);
    p += (m_numRules + 1)*sizeof(uint16);
    const int16 * const states = reinterpret_cast<const int16 *>(p);
    p += m_sTransition*m_sColumns*sizeof(int16);
    p += sizeof(byte);          // skip reserved byte
    if (p != pcCode || p >= pass_end) return false;
    p += pass_constraint_len;
    if (p != rcCode || p >= pass_end) return false;
    p += swap16(o_constraint[m_numRules]);
    if (p != aCode || p >= pass_end) return false;
    if (size_t(rcCode - pcCode) != pass_constraint_len) return false;

    // Load the pass constraint if there is one.
    if (pass_constraint_len)
    {
        m_cPConstraint = vm::Code(true, pcCode, pcCode + pass_constraint_len, 
                                  precontext[0], swap16(sort_keys[0]), *m_silf, face);
        if (!m_cPConstraint) return false;
    }
    if (!readRanges(ranges, numRanges)) return false;
    if (!readRules(rule_map, numEntries,  precontext, sort_keys,
                   o_constraint, rcCode, o_actions, aCode, face)) return false;
    return readStates(start_states, states, o_rule_map);
}


bool Pass::readRules(const uint16 * rule_map, const size_t num_entries,
                     const byte *precontext, const uint16 * sort_key,
                     const uint16 * o_constraint, const byte *rc_data,
                     const uint16 * o_action,     const byte * ac_data,
                     const Face & face)
{
    const byte * const ac_data_end = ac_data + swap16(o_action[m_numRules]);
    const byte * const rc_data_end = rc_data + swap16(o_constraint[m_numRules]);

    m_rules = gralloc<Rule>(m_numRules);
    precontext += m_numRules;
    sort_key   += m_numRules;
    o_constraint += m_numRules;
    o_action += m_numRules;

    // Load rules.
    const byte * ac_begin = 0, * rc_begin = 0,
               * ac_end = ac_data + swap16(*o_action),
               * rc_end = rc_data + swap16(*o_constraint);
    Rule * r = m_rules + m_numRules - 1;
    for (size_t n = m_numRules; n; --n, --r, ac_end = ac_begin, rc_end = rc_begin)
    {
        r->preContext = *--precontext;
        r->sort       = swap16(*--sort_key);
#ifndef NDEBUG
        r->rule_idx   = n;
#endif
        if (r->sort > 63 || r->preContext >= r->sort || r->preContext > m_maxPreCtxt || r->preContext < m_minPreCtxt)
            return false;
        ac_begin      = ac_data + swap16(*--o_action);
        rc_begin      = *--o_constraint ? rc_data + swap16(*o_constraint) : rc_end;

        if (ac_begin > ac_end || ac_begin > ac_data_end || ac_end > ac_data_end
                || rc_begin > rc_end || rc_begin > rc_data_end || rc_end > rc_data_end)
            return false;
        r->action     = new vm::Code(false, ac_begin, ac_end, r->preContext, r->sort, *m_silf, face);
        r->constraint = new vm::Code(true,  rc_begin, rc_end, r->preContext, r->sort, *m_silf, face);

        if (!r->action || !r->constraint
                || r->action->status() != Code::loaded
                || r->constraint->status() != Code::loaded
                || !r->constraint->immutable())
            return false;

        logRule(r, sort_key);
    }

    // Load the rule entries map
    RuleEntry * re = m_ruleMap = gralloc<RuleEntry>(num_entries);
    for (size_t n = num_entries; n; --n, ++re)
    {
        const ptrdiff_t rn = swap16(*rule_map++);
        if (rn >= m_numRules)  return false;
        re->rule = m_rules + rn;
    }

    return true;
}

static int cmpRuleEntry(const void *a, const void *b) { return (*(RuleEntry *)a < *(RuleEntry *)b ? -1 :
                                                                (*(RuleEntry *)b < *(RuleEntry *)a ? 1 : 0)); }

bool Pass::readStates(const int16 * starts, const int16 *states, const uint16 * o_rule_map)
{
    m_startStates = gralloc<State *>(m_maxPreCtxt - m_minPreCtxt + 1);
    m_states      = gralloc<State>(m_sRows);
    m_sTable      = gralloc<State *>(m_sTransition * m_sColumns);

    // load start states
    for (State * * s = m_startStates,
            * * const s_end = s + m_maxPreCtxt - m_minPreCtxt + 1; s != s_end; ++s)
    {
        *s = m_states + swap16(*starts++);
        if (*s < m_states || *s >= m_states + m_sRows) return false; // true;
    }

    // load state transition table.
    for (State * * t = m_sTable,
               * * const t_end = t + m_sTransition*m_sColumns; t != t_end; ++t)
    {
        *t = m_states + swap16(*states++);
        if (*t < m_states || *t >= m_states + m_sRows) return false;
    }

    State * s = m_states,
          * const transitions_end = m_states + m_sTransition,
          * const success_begin = m_states + m_sRows - m_sSuccess;
    const RuleEntry * rule_map_end = m_ruleMap + swap16(o_rule_map[m_sSuccess]);
    for (size_t n = m_sRows; n; --n, ++s)
    {
        s->transitions = s < transitions_end ? m_sTable + (s-m_states)*m_sColumns : 0;
        RuleEntry * const begin = s < success_begin ? 0 : m_ruleMap + swap16(*o_rule_map++),
                  * const end   = s < success_begin ? 0 : m_ruleMap + swap16(*o_rule_map);

        if (begin >= rule_map_end || end > rule_map_end || begin > end)
            return false;

        s->rules = begin;
        s->rules_end = end;
        qsort(begin, end - begin, sizeof(RuleEntry), &cmpRuleEntry);
    }

    logStates();
    return true;
}


void Pass::logRule(GR_MAYBE_UNUSED const Rule * r, GR_MAYBE_UNUSED const uint16 * sort_key) const
{
#ifndef DISABLE_TRACING
    if (!XmlTraceLog::get().active()) return;

    const size_t lid = r - m_rules;
    if (r->constraint)
    {
        XmlTraceLog::get().openElement(ElementConstraint);
        XmlTraceLog::get().addAttribute(AttrIndex, lid);
        XmlTraceLog::get().closeElement(ElementConstraint);
    }
    else
    {
        XmlTraceLog::get().openElement(ElementRule);
        XmlTraceLog::get().addAttribute(AttrIndex, lid);
        XmlTraceLog::get().addAttribute(AttrSortKey, swap16(sort_key[lid]));
        XmlTraceLog::get().addAttribute(AttrPrecontext, r->preContext);
        XmlTraceLog::get().closeElement(ElementRule);
    }
#endif
}

void Pass::logStates() const
{
#ifndef DISABLE_TRACING
    if (XmlTraceLog::get().active())
    {
        for (int i = 0; i != (m_maxPreCtxt - m_minPreCtxt + 1); ++i)
        {
            XmlTraceLog::get().openElement(ElementStartState);
            XmlTraceLog::get().addAttribute(AttrContextLen, i + m_minPreCtxt);
            XmlTraceLog::get().addAttribute(AttrState, size_t(m_startStates[i] - m_states));
            XmlTraceLog::get().closeElement(ElementStartState);
        }

        for (uint16 i = 0; i != m_sSuccess; ++i)
        {
            XmlTraceLog::get().openElement(ElementRuleMap);
            XmlTraceLog::get().addAttribute(AttrSuccessId, i);
            for (const RuleEntry *j = m_states[i].rules, *const j_end = m_states[i].rules_end; j != j_end; ++j)
            {
                XmlTraceLog::get().openElement(ElementRule);
                XmlTraceLog::get().addAttribute(AttrRuleId, size_t(j->rule - m_rules));
                XmlTraceLog::get().closeElement(ElementRule);
            }
            XmlTraceLog::get().closeElement(ElementRuleMap);
        }

        XmlTraceLog::get().openElement(ElementStateTransitions);
        for (size_t iRow = 0; iRow < m_sTransition; iRow++)
        {
            XmlTraceLog::get().openElement(ElementRow);
            XmlTraceLog::get().addAttribute(AttrIndex, iRow);
            const State * const * const row = m_sTable + iRow * m_sColumns;
            for (int i = 0; i != m_sColumns; ++i)
            {
                XmlTraceLog::get().openElement(ElementData);
                XmlTraceLog::get().addAttribute(AttrIndex, i);
                XmlTraceLog::get().addAttribute(AttrValue, size_t(row[i] - m_states));
                XmlTraceLog::get().closeElement(ElementData);
            }
            XmlTraceLog::get().closeElement(ElementRow);
        }
        XmlTraceLog::get().closeElement(ElementStateTransitions);
    }
#endif
}

bool Pass::readRanges(const uint16 *ranges, size_t num_ranges)
{
    m_cols = gralloc<uint16>(m_numGlyphs);
    memset(m_cols, 0xFF, m_numGlyphs * sizeof(uint16));
    for (size_t n = num_ranges; n; --n)
    {
        const uint16 first = swap16(*ranges++),
                     last  = swap16(*ranges++),
                     col   = swap16(*ranges++);
        uint16 *p;

        if (first > last || last >= m_numGlyphs || col >= m_sColumns)
            return false;

        for (p = m_cols + first; p <= m_cols + last; )
            *p++ = col;
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
    }
    return true;
}

void Pass::runGraphite(Machine & m, FiniteStateMachine & fsm) const
{
    Slot *s = m.slotMap().segment.first();
    if (!s || !testPassConstraint(m)) return;

    int hurdle=0, pos = 0, lc = m_iMaxLoop;
    do
    {
        pos += findNDoRule(s, m, fsm);
        if (pos > hurdle) {
            lc = m_iMaxLoop;
            hurdle = pos;
        }
        else if (--lc == 0)
        {
            while (pos <= hurdle && s) {
                s = s->next();
                ++pos;
            }
            hurdle = pos;
            lc = m_iMaxLoop;
        }
    } while (s);
}

inline uint16 Pass::glyphToCol(const uint16 gid) const
{
    return gid < m_numGlyphs ? m_cols[gid] : 0xffffU;
}

bool Pass::runFSM(FiniteStateMachine& fsm, Slot * slot) const
{
    int context = 0;
    for (; context != m_maxPreCtxt && slot->prev(); ++context, slot = slot->prev());
    if (context < m_minPreCtxt)
        return false;

    fsm.setContext(context);
    const State * state = m_startStates[m_maxPreCtxt - context];
    do
    {
        fsm.slots.pushSlot(slot);
        const uint16 col = glyphToCol(slot->gid());
        if (col == 0xffffU) return true;

        state = state->transitions[col];
        if (state->is_success())
            fsm.rules.accumulate_rules(*state);

#ifdef ENABLE_DEEP_TRACING
        if (col >= m_sColumns && col != 65535)
        {
            XmlTraceLog::get().error("Invalid column %d ID %d for slot %d",
                                     col, slot->gid(), slot);
        }
#endif

        slot = slot->next();
    } while (state->is_transition() && state != m_states && slot);

    fsm.slots.pushSlot(slot);
    return true;
}

int Pass::findNDoRule(Slot * & slot, Machine &m, FiniteStateMachine & fsm) const
{
    assert(slot);

    if (runFSM(fsm, slot))
    {
        // Search for the first rule which passes the constraint
        const RuleEntry *        r = fsm.rules.begin(),
                        * const re = fsm.rules.end();
        for (; r != re && !testConstraint(*r->rule, m); ++r);

        if (r != re)
        {
#ifdef ENABLE_DEEP_TRACING
            if (XmlTraceLog::get().active())
            {
                XmlTraceLog::get().openElement(ElementDoRule);
                XmlTraceLog::get().addAttribute(AttrNum, size_t(r->rule - m_rules));
                XmlTraceLog::get().addAttribute(AttrIndex, int(slot - fsm.slots.segment.first()));
            }
#endif
            const int res = doAction(r->rule->action, slot, m);
#ifdef ENABLE_DEEP_TRACING
            if (XmlTraceLog::get().active())
            {
                XmlTraceLog::get().openElement(ElementPassResult);
                XmlTraceLog::get().addAttribute(AttrResult, int(slot - fsm.slots.segment.first()));
                XmlTraceLog::get().closeElement(ElementPassResult);
                XmlTraceLog::get().closeElement(ElementDoRule);
            }
#endif
            return res;
        }
    }

    slot = slot->next();
    return 1;
}


inline
bool Pass::testPassConstraint(Machine & m) const
{
    if (!m_cPConstraint) return true;

    assert(m_cPConstraint.constraint());

    vm::slotref * map = m.slotMap().begin();
    *map = m.slotMap().segment.first();
    Machine::status_t status = Machine::finished;
    const uint32 ret = m_cPConstraint.run(m, map, status);

    return ret || status != Machine::finished;
}


bool Pass::testConstraint(const Rule &r, Machine & m) const
{
    if (r.sort > m.slotMap().size())    return false;
    if (!*r.constraint)                 return true;
    assert(r.constraint->constraint());

#ifdef ENABLE_DEEP_TRACING
    if (XmlTraceLog::get().active())
    {
        XmlTraceLog::get().openElement(ElementTestRule);
        XmlTraceLog::get().addAttribute(AttrNum, size_t(&r - m_rules));
    }
#endif
    vm::slotref * map = m.slotMap().begin() + m.slotMap().context() - r.preContext;
    Machine::status_t status = Machine::finished;
    for (int n = r.sort; n && *map; --n, ++map)
    {
        const int32 ret = r.constraint->run(m, map, status);
        if (!ret || status != Machine::finished)
        {
#ifdef ENABLE_DEEP_TRACING
            if (XmlTraceLog::get().active())
            {
                XmlTraceLog::get().closeElement(ElementTestRule);
            }
#endif
            return false;
        }
    }

#ifdef ENABLE_DEEP_TRACING
    if (XmlTraceLog::get().active())
    {
        XmlTraceLog::get().closeElement(ElementTestRule);
    }
#endif
    return true;
}


int Pass::doAction(const Code *codeptr, Slot * & slot_out, vm::Machine & m) const
{
    assert(codeptr && *codeptr);
    SlotMap   & smap = m.slotMap();
    vm::slotref * map = &smap[smap.context()];

    Segment & seg = smap.segment;
    int glyph_diff = -static_cast<int>(seg.slotCount());
    Machine::status_t status;
    int32 ret = codeptr->run(m, map, status);
    glyph_diff += seg.slotCount();
    if (codeptr->deletes())
    {
        for (Slot **s = smap.begin(), *const * const se = smap.end()-1; s != se; ++s)
        {
            Slot * & slot = *s;
            if (slot->isDeleted() || slot->isCopied()) seg.freeSlot(slot);
        }
    }

    int count   = map - smap.begin() - smap.context() + glyph_diff + ret;
    slot_out = *map;
    if (ret < 0)
    {
        if (!slot_out)
        {
            slot_out = seg.last();
            --count;
            ++ret;
        }
        while (++ret <= 0 && slot_out)
        {
            slot_out = slot_out->prev();
        }
    }
    else if (ret > 0)
    {
        assert(slot_out);
        if (!slot_out)
        {
            slot_out = seg.first();
            ++count;
            --ret;
        }
        while (--ret >= 0 && slot_out)
        {
            slot_out = slot_out->next();
        }
    }
    if (status != Machine::finished && slot_out) slot_out = NULL;

    return count;
}
