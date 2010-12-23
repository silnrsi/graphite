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
#pragma once

#include <cstdlib>
#include "Code.h"

class Segment;
class Face;
class Silf;
struct Rule;
struct RuleEntry;
struct State;
class FiniteStateMachine;

class Pass
{   
public:
    Pass();
    ~Pass();
    
    bool readPass(void* pPass, size_t pass_length, size_t subtable_base);
    void runGraphite(vm::Machine & m, FiniteStateMachine & fsm) const;
    void init(Silf *silf) { m_silf = silf; }

    CLASS_NEW_DELETE
private:
    int    findNDoRule(Slot* & iSlot, vm::Machine &, FiniteStateMachine& fsm) const;
    int    doAction(const vm::Code* codeptr, Slot * & slot_out, vm::Machine &) const;
    bool   testPassConstraint(vm::Machine & m) const;
    bool   testConstraint(const Rule & r, vm::Machine &) const;
    bool   readFSM(const byte* p, const byte*const pass_start, const size_t max_offset);
    bool   readRules(const uint16 * rule_map, const size_t num_entries, 
                     const byte *precontext, const uint16 * sort_key,
                     const uint16 * o_constraint, const byte *constraint_data, 
                     const uint16 * o_action, const byte * action_data);
    void   logRule(const Rule * r, const uint16 * sort_key) const;
    bool   readStates(const int16 * starts, const int16 * states, const uint16 * o_rule_map);
    void   logStates() const;
    bool   readRanges(const uint16* ranges, size_t num_ranges);
    uint16 glyphToCol(const uint16 gid) const;
    bool   runFSM(FiniteStateMachine & fsm, Slot * slot) const;
    
    const Silf* m_silf;
    uint16    * m_cols;
    Rule      * m_rules; // rules
    RuleEntry * m_ruleMap;
    State *   * m_startStates; // prectxt length
    State *   * m_sTable;
    State     * m_states;
    
    bool   m_immutable;
    byte   m_iMaxLoop;
    uint16 m_numGlyphs;
    uint16 m_numRules;
    uint16 m_sRows;
    uint16 m_sTransition;
    uint16 m_sSuccess;
    uint16 m_sColumns;
    byte m_minPreCtxt;
    byte m_maxPreCtxt;
    vm::Code m_cPConstraint;
    
private:		//defensive
    Pass(const Pass&);
    Pass& operator=(const Pass&);
};

