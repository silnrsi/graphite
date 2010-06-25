#ifndef VMSCRATCH_INCLUDE
#define VMSCRATCH_INCLUDE

#include <string.h>

#define MAX_RULES_PER_SEQUENCE 64
#define MAX_STACK 64

class VMScratch
{
public:
    void resetRules() { m_numRules = 0; }
    void resetStack() { m_stackptr = 0; }
    int32 *stack() { return m_stack; }
    uint16 rule(int i) { return m_rules[i]; }
    uint16 ruleLength() { return m_numRules; }
    void addRule(uint16 ruleid, uint16 sortkey) {
        int i;
        for (i = 0; i < m_numRules; i++)
        {
            if (m_rules[i] == ruleid) return;
            if (m_sortKeys[i] < sortkey) break;
        }
        memcpy(m_rules + i + 1, m_rules + i, (m_numRules - i) * sizeof(uint16));
        memcpy(m_sortKeys + i + 1, m_sortKeys + i, (m_numRules - i) * sizeof(uint16));
        m_rules[i] = ruleid;
        m_sortKeys[i] = sortkey;
        if (m_numRules < MAX_RULES_PER_SEQUENCE) m_numRules++;
    }
    void push(uint32 val) { if (m_stackptr < MAX_STACK) m_stack[m_stackptr++] = val; }
    uint16 pop() { return m_stackptr > 0 ? m_stack[m_stackptr--] : 0; }

protected:
    byte m_numRules;
    uint16 m_rules[MAX_RULES_PER_SEQUENCE];
    uint16 m_sortKeys[MAX_RULES_PER_SEQUENCE];
    byte m_stackptr;
    int32 m_stack[MAX_STACK];
};

#endif

