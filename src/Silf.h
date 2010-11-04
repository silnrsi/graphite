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

#include "Main.h"

#include "Pass.h"

namespace org { namespace sil { namespace graphite { namespace v2 {

class GrFace;
class VMScratch;
class GrSegment;
class SegCache;
class SegCacheEntry;

class Pseudo
{
public:
    uint32 uid;
    uint32 gid;
    CLASS_NEW_DELETE
};

class Silf
{
public:
    Silf() throw();
    ~Silf() throw();
    
    bool readGraphite(void *pSilf, size_t lSilf, int numGlyphs, uint32 version);
    void runGraphite(GrSegment *seg, const GrFace *face, VMScratch *vms) const;
    uint16 findClassIndex(uint16 cid, uint16 gid) const;
    uint16 getClassGlyph(uint16 cid, int index) const;
    uint16 findPseudo(uint32 uid) const;
    uint8 numUser() const { return m_aUser; }
    uint8 aPseudo() const { return m_aPseudo; }
    uint8 aBreak() const { return m_aBreak; }
    void enableSegmentCache(const GrFace *face, size_t maxSegments, uint32 flags);

    CLASS_NEW_DELETE

private:
    size_t readClassMap(void *pClass, size_t lClass, int numGlyphs);
    void runGraphiteWithCache(GrSegment *seg, const GrFace *face, VMScratch *vms) const;
    SegCacheEntry * runGraphiteOnSubSeg(GrSegment *seg, const GrFace *face, VMScratch *vms,
                             const Slot * firstSlot, size_t offset, size_t length) const;

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

    mutable SegCache * m_segCache;
    
    void releaseBuffers() throw();
    
private:			//defensive
    Silf(const Silf&);
    Silf& operator=(const Silf&);
};

}}}} // namespace
