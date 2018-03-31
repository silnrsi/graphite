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
#pragma once

#include <cstdlib>
#include "inc/Code.h"

namespace graphite2 {

class Segment;
class Face;
class Silf;
struct Rule;
struct RuleEntry;
struct State;
class FiniteStateMachine;
class Error;
class ShiftCollider;
class KernCollider;
class json;

enum passtype;

class Pass
{
public:
    Pass();
    ~Pass();

    bool readPass(const uint8_t * pPass, size_t pass_length, size_t subtable_base, Face & face,
        enum passtype pt, uint32_t version, Error &e);
    bool runGraphite(vm::Machine & m, FiniteStateMachine & fsm, bool reverse) const;
    void init(Silf *silf) { m_silf = silf; }
    uint8_t collisionLoops() const { return m_numCollRuns; }
    bool reverseDir() const { return m_isReverseDir; }

    CLASS_NEW_DELETE
private:
    void    findNDoRule(Slot* & iSlot, vm::Machine &, FiniteStateMachine& fsm) const;
    int     doAction(const vm::Machine::Code* codeptr, Slot * & slot_out, vm::Machine &) const;
    bool    testPassConstraint(vm::Machine & m) const;
    bool    testConstraint(const Rule & r, vm::Machine &) const;
    bool    readRules(const uint8_t * rule_map, const size_t num_entries,
                     const uint8_t *precontext, const uint16_t * sort_key,
                     const uint16_t * o_constraint, const uint8_t *constraint_data,
                     const uint16_t * o_action, const uint8_t * action_data,
                     Face &, enum passtype pt, Error &e);
    bool    readStates(const uint8_t * starts, const uint8_t * states, const uint8_t * o_rule_map, Face &, Error &e);
    bool    readRanges(const uint8_t * ranges, size_t num_ranges, Error &e);
    uint16_t  glyphToCol(const uint16_t gid) const;
    bool    runFSM(FiniteStateMachine & fsm, Slot * slot) const;
    void    dumpRuleEventConsidered(const FiniteStateMachine & fsm, const RuleEntry & re) const;
    void    dumpRuleEventOutput(const FiniteStateMachine & fsm, const Rule & r, Slot * os) const;
    void    adjustSlot(int delta, Slot * & slot_out, SlotMap &) const;
    bool    collisionShift(Segment *seg, int dir, json * const dbgout) const;
    bool    collisionKern(Segment *seg, int dir, json * const dbgout) const;
    bool    collisionFinish(Segment *seg, GR_MAYBE_UNUSED json * const dbgout) const;
    bool    resolveCollisions(Segment *seg, Slot *slot, Slot *start, ShiftCollider &coll, bool isRev,
                     int dir, bool &moved, bool &hasCol, json * const dbgout) const;
    float   resolveKern(Segment *seg, Slot *slot, Slot *start, int dir,
                     float &ymin, float &ymax, json *const dbgout) const;

    const Silf        * m_silf;
    uint16_t            * m_cols;
    Rule              * m_rules; // rules
    RuleEntry         * m_ruleMap;
    uint16_t            * m_startStates; // prectxt length
    uint16_t            * m_transitions;
    State             * m_states;
    vm::Machine::Code * m_codes;
    uint8_t              * m_progs;

    uint8_t   m_numCollRuns;
    uint8_t   m_kernColls;
    uint8_t   m_iMaxLoop;
    uint16_t m_numGlyphs;
    uint16_t m_numRules;
    uint16_t m_numStates;
    uint16_t m_numTransition;
    uint16_t m_numSuccess;
    uint16_t m_successStart;
    uint16_t m_numColumns;
    uint8_t m_minPreCtxt;
    uint8_t m_maxPreCtxt;
    uint8_t m_colThreshold;
    bool m_isReverseDir;
    vm::Machine::Code m_cPConstraint;

private:        //defensive
    Pass(const Pass&);
    Pass& operator=(const Pass&);
};

} // namespace graphite2
