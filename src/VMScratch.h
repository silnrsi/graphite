#pragma once

#include <string.h>

namespace org { namespace sil { namespace graphite { namespace v2 {

#define VMS_MAX_RULES_PER_SEQUENCE 64
#define VMS_MAX_STACK 64

class VMScratch
{
public:
    VMScratch() : m_firstPositioned(-1), m_lastPositioned(-1), m_numRules(0), m_stackptr(0) {}
    void resetRules() { m_numRules = 0; }
    void resetStack() { m_stackptr = 0; }
    int32 *stack() { return m_stack; }
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
    void push(uint32 val) { if (m_stackptr < VMS_MAX_STACK) m_stack[m_stackptr++] = val; }
    uint16 pop() { return m_stackptr > 0 ? m_stack[m_stackptr--] : 0; }

protected:
    byte m_numRules;
    byte m_stackptr;
    uint32 m_firstPositioned;
    uint32 m_lastPositioned;
    uint16 m_rules[VMS_MAX_RULES_PER_SEQUENCE];
    uint16 m_sortKeys[VMS_MAX_RULES_PER_SEQUENCE];
    int16 m_lengths[VMS_MAX_RULES_PER_SEQUENCE];
    int32 m_stack[VMS_MAX_STACK];
};

}}}} // namespace
