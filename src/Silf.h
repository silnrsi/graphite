#pragma once

#include "Main.h"

#include "Pass.h"
class GrFace;
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
    Silf() throw();
    ~Silf() throw();
    
    bool readGraphite(void *pSilf, size_t lSilf, int numGlyphs, uint32 version);
    void runGraphite(Segment *seg, const GrFace *face, VMScratch *vms) const;
    uint16 findClassIndex(uint16 cid, uint16 gid) const;
    uint16 getClassGlyph(uint16 cid, int index) const;
    uint16 findPseudo(uint32 uid) const;
    uint8 numUser() const { return m_aUser; }
    uint8 aPseudo() const { return m_aPseudo; }
    uint8 aBreak() const { return m_aBreak; }

private:
    size_t readClassMap(void *pClass, size_t lClass, int numGlyphs);

    Pass          * m_passes;
    Pseudo        * m_pseudos;
    uint16        * m_classOffsets, 
                  * m_classData;
    size_t          m_numPasses;
    uint8           m_sPass, m_pPass, m_jPass, m_bPass,
                    m_flags;

    uint8   m_aPseudo, m_aBreak, m_aUser, 
            m_iMaxComp;
    uint16  m_aLig,
            m_numPseudo,
            m_nClass,
            m_nLinear;
    
    void releaseBuffers() throw();
    
private:			//defensive
    Silf(const Silf&);
    Silf& operator=(const Silf&);
};

