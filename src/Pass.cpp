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
    If not, write to the Free Software Foundation, 51 Franklin Street,
    Suite 500, Boston, MA 02110-1335, USA or visit their web page on the
    internet at http://www.fsf.org/licenses/lgpl.html.

Alternatively, the contents of this file may be used under the terms of the
Mozilla Public License (http://mozilla.org/MPL) or the GNU General Public
License, as published by the Free Software Foundation, either version 2
of the License or (at your option) any later version.
*/
#include "Main.h"
#include "debug.h"
#include "Endian.h"
#include "Pass.h"
#include <cstring>
#include <cstdlib>
#include <cassert>
#include "Segment.h"
#include "Code.h"
#include "Rule.h"

using namespace graphite2;
using vm::Machine;
typedef Machine::Code  Code;


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

    delete [] m_rules;
}

bool Pass::readPass(void *pass, size_t pass_length, size_t subtable_base, const Face & face)
{
    const byte *                p = reinterpret_cast<const byte *>(pass),
               * const pass_start = p,
               * const pass_end   = p + pass_length;
    size_t numRanges;

    if (pass_length < 40) return false; 
    // Read in basic values
    m_immutable = (*p++) & 0x1U;
    m_iMaxLoop = *p++;
    p++; // skip maxContext
    p += sizeof(byte);     // skip maxBackup
    m_numRules = be::read<uint16>(p);
    p += sizeof(uint16);   // not sure why we would want this
    const byte * const pcCode = pass_start + be::read<uint32>(p) - subtable_base,
               * const rcCode = pass_start + be::read<uint32>(p) - subtable_base,
               * const aCode  = pass_start + be::read<uint32>(p) - subtable_base;
    p += sizeof(uint32);
    m_sRows = be::read<uint16>(p);
    m_sTransition = be::read<uint16>(p);
    m_sSuccess = be::read<uint16>(p);
    m_sColumns = be::read<uint16>(p);
    numRanges = be::read<uint16>(p);
    p += sizeof(uint16)   // skip searchRange
         +  sizeof(uint16)   // skip entrySelector
         +  sizeof(uint16);  // skip rangeShift
    assert(p - pass_start == 40);
    // Perform some sanity checks.
    if (   m_sTransition > m_sRows
            || m_sSuccess > m_sRows
            || m_sSuccess + m_sTransition < m_sRows)
        return false;

    if (p + numRanges * 6 - 4 > pass_end) return false;
    m_numGlyphs = be::swap<uint16>(*(uint16 *)(p + numRanges * 6 - 4)) + 1;
    // Caculate the start of vairous arrays.
    const uint16 * const ranges = reinterpret_cast<const uint16 *>(p);
    p += numRanges*sizeof(uint16)*3;
    const uint16 * const o_rule_map = reinterpret_cast<const uint16 *>(p);
    p += (m_sSuccess + 1)*sizeof(uint16);

    // More sanity checks
    if (   reinterpret_cast<const byte *>(o_rule_map) > pass_end
            || p > pass_end)
        return false;
    const size_t numEntries = be::swap<uint16>(o_rule_map[m_sSuccess]);
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
    const size_t pass_constraint_len = be::read<uint16>(p);
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
    p += be::swap<uint16>(o_constraint[m_numRules]);
    if (p != aCode || p >= pass_end) return false;
    if (size_t(rcCode - pcCode) != pass_constraint_len) return false;

    // Load the pass constraint if there is one.
    if (pass_constraint_len)
    {
        m_cPConstraint = vm::Machine::Code(true, pcCode, pcCode + pass_constraint_len, 
                                  precontext[0], be::swap<uint16>(sort_keys[0]), *m_silf, face);
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
    const byte * const ac_data_end = ac_data + be::swap<uint16>(o_action[m_numRules]);
    const byte * const rc_data_end = rc_data + be::swap<uint16>(o_constraint[m_numRules]);

    if (!(m_rules = new Rule [m_numRules])) return false;
    precontext += m_numRules;
    sort_key   += m_numRules;
    o_constraint += m_numRules;
    o_action += m_numRules;

    // Load rules.
    const byte * ac_begin = 0, * rc_begin = 0,
               * ac_end = ac_data + be::swap<uint16>(*o_action),
               * rc_end = rc_data + be::swap<uint16>(*o_constraint);
    Rule * r = m_rules + m_numRules - 1;
    for (size_t n = m_numRules; n; --n, --r, ac_end = ac_begin, rc_end = rc_begin)
    {
        r->preContext = *--precontext;
        r->sort       = be::swap<uint16>(*--sort_key);
#ifndef NDEBUG
        r->rule_idx   = n - 1;
#endif
        if (r->sort > 63 || r->preContext >= r->sort || r->preContext > m_maxPreCtxt || r->preContext < m_minPreCtxt)
            return false;
        ac_begin      = ac_data + be::swap<uint16>(*--o_action);
        rc_begin      = *--o_constraint ? rc_data + be::swap<uint16>(*o_constraint) : rc_end;

        if (ac_begin > ac_end || ac_begin > ac_data_end || ac_end > ac_data_end
                || rc_begin > rc_end || rc_begin > rc_data_end || rc_end > rc_data_end)
            return false;
        r->action     = new vm::Machine::Code(false, ac_begin, ac_end, r->preContext, r->sort, *m_silf, face);
        r->constraint = new vm::Machine::Code(true,  rc_begin, rc_end, r->preContext, r->sort, *m_silf, face);

        if (!r->action || !r->constraint
                || r->action->status() != Code::loaded
                || r->constraint->status() != Code::loaded
                || !r->constraint->immutable())
            return false;
    }

    // Load the rule entries map
    RuleEntry * re = m_ruleMap = gralloc<RuleEntry>(num_entries);
    for (size_t n = num_entries; n; --n, ++re)
    {
        const ptrdiff_t rn = be::swap<uint16>(*rule_map++);
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

    if (!m_startStates || !m_states || !m_sTable) return false;
    // load start states
    for (State * * s = m_startStates,
            * * const s_end = s + m_maxPreCtxt - m_minPreCtxt + 1; s != s_end; ++s)
    {
        *s = m_states + be::swap<uint16>(*starts++);
        if (*s < m_states || *s >= m_states + m_sRows) return false; // true;
    }

    // load state transition table.
    for (State * * t = m_sTable,
               * * const t_end = t + m_sTransition*m_sColumns; t != t_end; ++t)
    {
        *t = m_states + be::swap<uint16>(*states++);
        if (*t < m_states || *t >= m_states + m_sRows) return false;
    }

    State * s = m_states,
          * const transitions_end = m_states + m_sTransition,
          * const success_begin = m_states + m_sRows - m_sSuccess;
    const RuleEntry * rule_map_end = m_ruleMap + be::swap<uint16>(o_rule_map[m_sSuccess]);
    for (size_t n = m_sRows; n; --n, ++s)
    {
        s->transitions = s < transitions_end ? m_sTable + (s-m_states)*m_sColumns : 0;
        RuleEntry * const begin = s < success_begin ? 0 : m_ruleMap + be::swap<uint16>(*o_rule_map++),
                  * const end   = s < success_begin ? 0 : m_ruleMap + be::swap<uint16>(*o_rule_map);

        if (begin >= rule_map_end || end > rule_map_end || begin > end)
            return false;
#ifndef NDEBUG
        s->index = (s - m_states);
#endif
        s->rules = begin;
        s->rules_end = (end - begin <= FiniteStateMachine::MAX_RULES)? end :
            begin + FiniteStateMachine::MAX_RULES;
        qsort(begin, end - begin, sizeof(RuleEntry), &cmpRuleEntry);
    }

    return true;
}

bool Pass::readRanges(const uint16 *ranges, size_t num_ranges)
{
    m_cols = gralloc<uint16>(m_numGlyphs);
    memset(m_cols, 0xFF, m_numGlyphs * sizeof(uint16));
    for (size_t n = num_ranges; n; --n)
    {
        const uint16 first = be::swap<uint16>(*ranges++),
                     last  = be::swap<uint16>(*ranges++),
                     col   = be::swap<uint16>(*ranges++);
        uint16 *p;

        if (first > last || last >= m_numGlyphs || col >= m_sColumns)
            return false;

        for (p = m_cols + first; p <= m_cols + last; )
            *p++ = col;
    }
    return true;
}


void Pass::runGraphite(Machine & m, FiniteStateMachine & fsm) const
{
	Slot *s = m.slotMap().segment.first();
	if (!s || !testPassConstraint(m)) return;
    Slot *currHigh = s->next();

    if (dbgout)  *dbgout << "rules"	<< json::array;
	json::closer rules_array_closer = dbgout;

    m.slotMap().highwater(currHigh);
    int lc = m_iMaxLoop;
    do
    {
        findNDoRule(s, m, fsm);
        if (s && (m.slotMap().highpassed() || s == m.slotMap().highwater() || --lc == 0)) {
        	if (!lc)
        	{
//        		if (dbgout)	*dbgout << json::item << json::flat << rule_event(-1, s, 1);
        		s = m.slotMap().highwater();
        	}
        	lc = m_iMaxLoop;
            if (s)
            	m.slotMap().highwater(s->next());
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
        if (fsm.slots.size() >= SlotMap::MAX_SLOTS) return false;
        const uint16 col = glyphToCol(slot->gid());
        if (col == 0xffffU || !state->is_transition()) return true;

        state = state->transitions[col];
        if (state->is_success())
            fsm.rules.accumulate_rules(*state);

        slot = slot->next();
    } while (state != m_states && slot);

    fsm.slots.pushSlot(slot);
    return true;
}

void Pass::findNDoRule(Slot * & slot, Machine &m, FiniteStateMachine & fsm) const
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
			const int adv = doAction(r->rule->action, slot, m);
			if (dbgout) dumpRuleEvent(fsm, *r, slot);
			adjustSlot(adv, slot, fsm.slots);

            return;
        }
    }

    slot = slot->next();
}


void Pass::dumpRuleEvent(const FiniteStateMachine & fsm, const RuleEntry & re, Slot * os) const
{
	os = os ? os->prev() : fsm.slots.segment.last();
	*dbgout << json::item << json::object
				<< "considered" << json::array;
				for (const RuleEntry *r = fsm.rules.begin(); r != &re; ++r)
	*dbgout 		<< json::flat << json::object
						<< "id" 	<< r->rule - m_rules
						<< "input" << json::flat << json::object
							<< "start" << slotid(fsm.slots[fsm.slots.context() - r->rule->preContext])
							<< "length" << r->rule->sort
							<< json::close	// close "input"
						<< json::close;	// close Rule object
	*dbgout 		<< json::flat << json::object
						<< "id" 	<< re.rule - m_rules
						<< "input" << json::flat << json::object
							<< "start" << slotid(fsm.slots[fsm.slots.context()])
							<< "length" << re.rule->sort - re.rule->preContext
							<< json::close // close "input"
						<< json::close	// close object
				<< json::close; // close considered array
	*dbgout 	<< "output"	<< json::array;
	for(int n = re.rule->sort - re.rule->preContext; n; --n, os = os->prev())
		*dbgout 	<< dslot(fsm.slots.segment,*os);
	*dbgout 		<< json::close // close "output"
				<< json::close;	// close RuleEvent object
}


inline
bool Pass::testPassConstraint(Machine & m) const
{
    if (!m_cPConstraint) return true;

    assert(m_cPConstraint.constraint());

    vm::slotref * map = m.slotMap().begin();
    *map = m.slotMap().segment.first();
    const uint32 ret = m_cPConstraint.run(m, map);

    if (dbgout)
    	*dbgout << "constraint" << (ret || m.status() != Machine::finished);

    return ret || m.status() != Machine::finished;
}


bool Pass::testConstraint(const Rule &r, Machine & m) const
{
    if ((r.sort - r.preContext) > (m.slotMap().size() - m.slotMap().context()))    return false;
    if (m.slotMap().context() - r.preContext < 0) return false;
    if (!*r.constraint)                 return true;
    assert(r.constraint->constraint());

    vm::slotref * map = m.slotMap().begin() + m.slotMap().context() - r.preContext;
    for (int n = r.sort; n && map; --n, ++map)
    {
    	if (!*map) continue;
        const int32 ret = r.constraint->run(m, map);
        if (!ret || m.status() != Machine::finished)
            return false;
    }

    return true;
}


int Pass::doAction(const Code *codeptr, Slot * & slot_out, vm::Machine & m) const
{
    assert(codeptr);
    if (!*codeptr) return 0;
    SlotMap   & smap = m.slotMap();
    vm::slotref * map = &smap[smap.context()];
    smap.highpassed(false);

    Segment & seg = smap.segment;
    int glyph_diff = -static_cast<int>(seg.slotCount());
    int32 ret = codeptr->run(m, map);
    glyph_diff += seg.slotCount();

    if (codeptr->deletes())
    {
        for (Slot **s = smap.begin(), *const * const se = smap.end()-1; s != se; ++s)
        {
            Slot * & slot = *s;
            if (slot->isDeleted() || slot->isCopied())
            {
            	if (slot == smap.highwater())
            		smap.highwater(slot->next());
            	seg.freeSlot(slot);
            }
        }
    }

    if (m.status() != Machine::finished)
    {
    	slot_out = NULL;
    	m.slotMap().highwater(0);
    	return 0;
    }

    slot_out = *map;
    return ret;
}

void Pass::adjustSlot(int delta, Slot * & slot_out, SlotMap & smap) const
{
    if (delta < 0)
    {
        if (!slot_out)
        {
            slot_out = smap.segment.last();
            ++delta;
            if (smap.highpassed() && !smap.highwater())
            	smap.highpassed(false);
        }
        while (++delta <= 0 && slot_out)
        {
            slot_out = slot_out->prev();
            if (smap.highpassed() && smap.highwater() == slot_out)
            	smap.highpassed(false);
        }
    }
    else if (delta > 0)
    {
        if (!slot_out)
        {
            slot_out = smap.segment.first();
            --delta;
        }
        while (--delta >= 0 && slot_out)
        {
            slot_out = slot_out->next();
            if (slot_out == smap.highwater() && slot_out)
                smap.highpassed(true);
        }
    }
}

