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
#include "VMScratch.h"
#include "Code.h"

namespace org { namespace sil { namespace graphite { namespace v2 {

class GrSegment;
class GrFace;
class Silf;

class Pass
{   
public:
    Pass();
    ~Pass();
    
    bool readPass(void *pPass, size_t lPass);
    int readCodePointers(byte *pCode, byte *pPointers, vm::Code *pRes, int num, bool isConstraint, vm::CodeContext *cContexts);
    void runGraphite(GrSegment *seg, const GrFace *face, VMScratch *vms) const;
    Slot *findNDoRule(GrSegment* seg, Slot* iSlot, int& count, const GrFace* face, VMScratch* vms) const;
    int testConstraint(const vm::Code* codeptr, Slot* iSlot, int num, int nPre, int nCtxt, GrSegment* seg, int nMap, Slot** map, int &flags) const;
    Slot *doAction(const vm::Code* codeptr, Slot* iSlot, int& count, int nPre, int len, GrSegment* seg, Slot** map, int &flags) const;
    void init(Silf *silf) { m_silf = silf; }

    CLASS_NEW_DELETE
private:
    Silf *m_silf;
    byte m_iMaxLoop;
    uint16 m_numGlyphs;
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
    vm::Code m_cPConstraint;
    vm::Code *m_cConstraint;
    vm::Code *m_cActions;
    int16 *m_sTable;
    
private:		//defensive
    Pass(const Pass&);
    Pass& operator=(const Pass&);
};

}}}} // namespace
