#ifndef SILF_INCLUDE
#define SILF_INCLUDE
#include "Main.h"

#include "Pass.h"
class ISegment;
class LoadedFace;
class FontImpl;
class VMScratch;
class Segment;

class Pseudo
{
public:
    uint32 uid;
    uint32 gid;
};

class Silf
{
public:
    bool readGraphite(void *pSilf, size_t lSilf, int numGlyphs, uint32 version);
    void runGraphite(Segment *seg, const LoadedFace *face, VMScratch *vms) const;
    uint16 findClassIndex(uint16 cid, uint16 gid);
    uint16 getClassGlyph(uint16 cid, uint16 index);
    uint16 findPseudo(uint32 uid);

protected:
    size_t readClassMap(void *pClass, size_t lClass, int numGlyphs);

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
    uint16 m_numPseudo;
    Pseudo *m_pseudos;
    uint16 m_nClass;
    uint16 m_nLinear;
    uint16 *m_classOffsets;
    uint16 *m_classData;

};

#endif
