#ifndef PASS_INCLUDE
#define PASS_INCLUDE

class Pass
{
public:
    bool readPass(void *pPass, size_t lPass, int numGlyphs);

protected:
    byte m_iMaxLoop;
    uint16 m_numRules;
    uint16 m_sRows;
    uint16 m_sTransition;
    uint16 m_sSuccess;
    uint16 m_sColumns;
    uint16 *m_cols;
    uint16 *m_ruleidx;
    uint16 *m_ruleMap;
    uint16 *m_ruleSorts;
    byte m_minPreCtxt;
    byte m_maxPreCtxt;
    uint16 *m_startStates;
    byte *m_rulePreCtxt;
    uint16 *m_pConstraint;
    uint16 *m_pActions;
    byte *m_cPConstraint;
    byte *m_cConstraint;
    byte *m_cActions;
    int16 *m_sTable;
};

#endif
