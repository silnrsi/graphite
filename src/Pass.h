#ifndef PASS_INCLUDE
#define PASS_INCLUDE

#include "VMScratch.h"

class Segment;
class FontImpl;
class FontFace;
class Silf;

class Pass
{
public:
    bool readPass(void *pPass, size_t lPass, int numGlyphs);
    void runGraphite(Segment *seg, FontFace *face, Silf *silf, VMScratch *vms);
    int findNDoRule(Segment *seg, int iSlot, VMScratch *vms, Silf *silf);
    int testConstraint(byte *codeptr, size_t codelen, int iSlot, Segment *seg, Silf *silf, VMScratch *vms);
    int doAction(byte *m_cAction, size_t numAction, int startSlot, Segment *seg, Silf *silf, VMScratch *vms);

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
    uint16 m_nPConstraint;
    uint16 *m_pConstraint;
    uint16 *m_pActions;
    byte *m_cPConstraint;
    byte *m_cConstraint;
    byte *m_cActions;
    int16 *m_sTable;
};

#endif
