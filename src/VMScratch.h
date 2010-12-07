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

#include <string.h>

namespace org { namespace sil { namespace graphite { namespace v2 {

class GrSlot;

#define VMS_MAX_RULES_PER_SEQUENCE 64

class VMScratch
{
public:
    VMScratch() : m_numRules(0), m_firstPositioned(-1), m_lastPositioned(-1) {}
    void resetRules() { m_numRules = 0; m_flags = 0; }
    void resetStack() { m_stackptr = 0; }
    uint16 rule(int i) { return m_rules[i]; }
    uint16 ruleLength() { return m_numRules; }
    int16 length(int i) { return m_lengths[i]; }
    void addRule(uint16 ruleid, uint16 sortkey, int16 len) {
        int i;
        for (i = 0; i < m_numRules; i++)
        {
            if (m_rules[i] == ruleid) return;
            if (m_sortKeys[i] < sortkey || (m_sortKeys[i] == sortkey && ruleid <= m_rules[i])) break;
        }
        memcpy(m_rules + i + 1, m_rules + i, (m_numRules - i) * sizeof(uint16));
        memcpy(m_sortKeys + i + 1, m_sortKeys + i, (m_numRules - i) * sizeof(uint16));
        memcpy(m_lengths + i + 1, m_lengths + i, (m_numRules - i) * sizeof(uint16));
        m_rules[i] = ruleid;
        m_sortKeys[i] = sortkey;
        m_lengths[i] = len;
        if (m_numRules < VMS_MAX_RULES_PER_SEQUENCE) m_numRules++;
    }
    GrSlot *slotMap(int i) { return m_slotMap[i]; }
    void slotMap(int i, GrSlot *s) { m_slotMap[i] = s; }
    GrSlot **map() { return m_slotMap; }
    byte flags() { return m_flags; }
    void setflag(int val) { m_flags |= val; }

protected:
    byte m_numRules;
    byte m_stackptr;
    byte m_flags;
    uint32 m_firstPositioned;
    uint32 m_lastPositioned;
    uint16 m_rules[VMS_MAX_RULES_PER_SEQUENCE];
    uint16 m_sortKeys[VMS_MAX_RULES_PER_SEQUENCE];
    int16 m_lengths[VMS_MAX_RULES_PER_SEQUENCE];
    GrSlot *m_slotMap[VMS_MAX_RULES_PER_SEQUENCE];
};

}}}} // namespace
