#ifndef SILF_INCLUDE
#define SILF_INCLUDE
#include "Main.h"

#include "Pass.h"
class ISegment;
class FontFace;
class FontImpl;
class VMScratch;
class Segment;

class Silf
{
public:
    bool readGraphite(void *pSilf, size_t lSilf, int numGlyphs, uint32 version);
    void runGraphite(Segment *seg, FontImpl *font, FontFace *face, VMScratch *vms);

protected:
    size_t readClassMap(void *pClass, size_t lClass);

    Pass *m_passes;
    byte m_numPasses;
    byte m_sPass;
    byte m_pPass;
    byte m_jPass;
    byte m_bPass;
    byte m_flags;

    byte m_aBreak;
    byte m_aUser;
    byte m_iMaxComp;
    uint16 m_aLig;

};

#endif
