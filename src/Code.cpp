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
// This class represents loaded graphite stack machine code.  It performs 
// basic sanity checks, on the incoming code to prevent more obvious problems
// from crashing graphite.
// Author: Tim Eves

#include <cassert>
#include <cstdlib>
#include <stdexcept>
#include <string.h>
#include "Code.h"
#include "Machine.h"
#include "XmlTraceLog.h"

#include <cstdio>

using namespace org::sil::graphite::v2;
using namespace vm;

namespace {

inline bool is_return(const opcode opc) {
    return opc == POP_RET || opc == RET_ZERO || opc == RET_TRUE;
}

void emit_trace_message(opcode, const byte *const, const opcode_t &);
void fixup_cntxt_item_target(const byte*, byte * &);

} // end namespace


Code::Code(bool constrained, const byte * bytecode_begin, const byte * const bytecode_end, CodeContext *cContexts)
 :  _code(0), _data_size(0), _instr_count(0), _status(loaded), 
    _constrained(constrained), _own(true)
{
    assert(bytecode_begin != 0);
    assert(bytecode_end > bytecode_begin);
    const opcode_t *    op_to_fn = Machine::getOpcodeTable();
    const byte *        cd_ptr = bytecode_begin;
    byte                iSlot = 0;
    
    // Allocate code and dat target buffers, these sizes are a worst case 
    // estimate.  Once we know their real sizes the we'll shrink them.
    _code = static_cast<instr *>(std::malloc((bytecode_end - bytecode_begin)
                                             * sizeof(instr)));
    _data = static_cast<byte *>(std::malloc((bytecode_end - bytecode_begin)
                                             * sizeof(byte)));
    
    if (!_code || !_data) {
        failure(alloc_failed);
        return;
    }
    
    instr * ip = _code;
    byte  * dp = _data;
    opcode  opc;
    cContexts[0] = CodeContext(0, 0, 0);
    do {
        opc = opcode(*cd_ptr++);
        
        // Do some basic sanity checks based on what we know about the opcodes.
        if (!check_opcode(opc, cd_ptr, bytecode_end))
            return;

        const opcode_t & op = op_to_fn[opc];
        if (op.impl[constrained] == 0) {      // Is it implemented?
            failure(unimplemented_opcode_used);
            return;
        }
        
        const size_t param_sz = op.param_sz == VARARGS ? *cd_ptr + 1 : op.param_sz;
        if (cd_ptr + param_sz > bytecode_end) { // Is the requested size possible
            failure(arguments_exhausted);
            return;
        }
        
        // Add this instruction
        *ip++ = op.impl[constrained]; 
        ++_instr_count;
        emit_trace_message(opc, cd_ptr, op);

        // Grab the parameters
        if (param_sz) {
            std::copy(cd_ptr, cd_ptr + param_sz, dp);
            cd_ptr += param_sz;
            dp     += param_sz;
        }
        
        // Fixups to any argument data that needs it.
        if (opc == CNTXT_ITEM)
            fixup_cntxt_item_target(cd_ptr, dp);
        if (!constrained)
            fixup_instruction_offsets(opc, _instr_count, reinterpret_cast<int8 *>(dp), param_sz, 
                                      iSlot, cContexts);
    } while (!is_return(opc) && cd_ptr < bytecode_end);
    
    // Final sanity check: ensure that the program is correctly terminated.
    if (!is_return(opc)) {
        failure(missing_return);
        return;
    }
    
    // insert TEMP_COPY commands for slots that need them (that change and are referenced later)
    if (!constrained)
        for (int i = iSlot - 1; i >= 0; i--)
            if (cContexts[i].copySlot == 3)
            {
                memmove(_code + cContexts[i].codeRef + 1, _code + cContexts[i].codeRef, (_instr_count - cContexts[i].codeRef) * sizeof(instr));
                _code[cContexts[i].codeRef] = op_to_fn[TEMP_COPY].impl[constrained];
                _instr_count++;
            }

//    assert(ip - _code == ptrdiff_t(_instr_count));
    _data_size = sizeof(byte)*(dp - _data);
    
    // Now we know exactly how much code and data the program really needs
    // realloc the buffers to exactly the right size so we don't waste any 
    // memory.
    assert((bytecode_end - bytecode_begin)*sizeof(instr) >= _instr_count*sizeof(instr));
    assert((bytecode_end - bytecode_begin)*sizeof(byte) >= _data_size*sizeof(byte));
    _code = static_cast<instr *>(std::realloc(_code, (_instr_count+1)*sizeof(instr)));
    _data = static_cast<byte *>(std::realloc(_data, (_data_size+1)*sizeof(byte)));
    // Make this RET_ZERO, we should never reach this but just in case ...
    _code[_instr_count] = op_to_fn[RET_ZERO].impl[constrained];
    _data[_data_size]   = 0;

    assert(_code);
    if (!_code) {
        failure(alloc_failed);
        return;
    }    
}

Code::~Code() throw ()
{
    if (_own)
        release_buffers();
}


// Validation check and fixups.
//
bool Code::check_opcode(const opcode opc, 
                        const byte *cd_ptr, 
                        const byte *const cd_end) 
{
    if (opc >= MAX_OPCODE) {   // Is this even a valid opcode?
        failure(invalid_opcode);
        return false;
    }
    
    if (opc == CNTXT_ITEM)  // This is a really conditional forward jump,
    {                       // check it doesn't jump outside the program.
        const size_t skip = cd_ptr[1];
        if (cd_ptr + 2 + skip > cd_end) {
            failure(jump_past_end);
            return false;
        }
    }
    return true;
}

namespace {

inline void emit_trace_message(opcode opc, const byte *const params, 
                        const opcode_t &op)
{
#ifndef DISABLE_TRACING
    if (XmlTraceLog::get().active())
    {
        XmlTraceLog::get().openElement(ElementAction);
        XmlTraceLog::get().addAttribute(AttrActionCode, opc);
        XmlTraceLog::get().addAttribute(AttrAction, op.name);
        for (size_t p = 0; p < 8 && p < op.param_sz; ++p)
        {
            XmlTraceLog::get().addAttribute(
                                XmlTraceLogAttribute(Attr0 + p),
                                params[p]);
        }
        XmlTraceLog::get().closeElement(ElementAction);
    }
#endif
}


void fixup_cntxt_item_target(const byte* cdp, 
                       byte * & dp) {
    
    const opcode_t    * oplut = Machine::getOpcodeTable();
    size_t              data_skip = 0;
    uint8               count = uint8(dp[-1]); 
    
    while (count > 0) {
        const opcode    opc      = opcode(*cdp++);
        size_t          param_sz = oplut[opc].param_sz;
        
        if (param_sz == VARARGS)    param_sz = *cdp+1;
        cdp       += param_sz;
        data_skip += param_sz +(opc == CNTXT_ITEM);
        count     -= 1 + param_sz;
    }
    assert(count == 0);
    dp[-1] -= data_skip;
    *dp++   = data_skip;
}

// inline void fixup_slotref(int8 * const arg, uint8 is, const CodeContext *const cContexts) {
//    if (*arg < 0 && -*arg <= is)
//        *arg -= cContexts[is + *arg].nInserts;
//    else
//        *arg += ctxtins(is);
// }

} // end of namespace

void Code::fixup_instruction_offsets(const opcode opc, size_t cp,
                                     int8  * dp, size_t param_sz,
                                     byte & iSlot, CodeContext* cContexts)
{
    
    switch (opc)
    {
        case NEXT :
        case COPY_NEXT :
            iSlot++;
            cContexts[iSlot] = CodeContext(0, 0, cp);
            break;
//         case INSERT :
//             for (int i = iSlot; i >= 0; --i)
//                 ++cContexts[i].nInserts;
//             break;
        case DELETE :
            iSlot--;
            break;
        case PUT_COPY :
            if (dp[-1] != 0) cContexts[iSlot].copySlot = 1;
            if (dp[-1] < 0 && -dp[-1] <= iSlot)
                cContexts[iSlot + dp[-1]].copySlot |= 2;
        case PUSH_SLOT_ATTR :
        case PUSH_GLYPH_ATTR_OBS :
        case PUSH_GLYPH_ATTR :
//             fixup_slotref(dp-1,iSlot,cContexts);
            if (dp[-1] <= 0 && -dp[-1] <= iSlot)
                cContexts[iSlot + dp[-1]].copySlot |= 2;
            break;
//         case PUSH_FEAT :
//         case PUSH_ATT_TO_GATTR_OBS :
//         case PUSH_ATT_TO_GLYPH_ATTR :
//             fixup_slotref(dp-1,iSlot,cContexts);
//             break;
        case PUSH_ISLOT_ATTR :
//            cContexts[iSlot].copySlot = 1;
//             fixup_slotref(dp-2,iSlot,cContexts);
            if (dp[-2] <= 0 && -dp[-2] <= iSlot)
                cContexts[iSlot + dp[-2]].copySlot |= 2;
            break;
//         case PUSH_GLYPH_METRIC :
//         case PUSH_ATT_TO_GLYPH_METRIC :
//             fixup_slotref(dp-2,iSlot,cContexts);
//             break;
        case PUT_SUBS_8BIT_OBS:
            cContexts[iSlot].copySlot = 1;
//             fixup_slotref(dp-3,iSlot,cContexts);
            if (dp[-3] <= 0 && -dp[-3] <= iSlot)
                cContexts[iSlot + dp[-3]].copySlot |= 2;
            break;
//         case CNTXT_ITEM :
//             fixup_slotref(dp-3,iSlot,cContexts);
// 	        break;
        case PUT_SUBS :
            cContexts[iSlot].copySlot = 1;
//             fixup_slotref(dp-5,iSlot,cContexts);
            if (dp[-5] <= 0 && -dp[-5] <= iSlot)
                cContexts[iSlot + dp[-5]].copySlot |= 2;
            break;
//         case ASSOC :
//             for (size_t i = 1; i < param_sz; ++i)
//                 fixup_slotref(dp-i,iSlot,cContexts);
//             break;
    }
}


inline 
void Code::failure(const status_t s) throw() {
    release_buffers();
    _status = s;
}

void Code::release_buffers() throw() 
{
    std::free(_code);
    std::free(_data);
    _code = 0;
    _data = 0;
    _own  = false;
}


int32 Code::run(Machine & m, GrSegment & seg, slotref & islot_idx, int &count, int &nPre, int maxmap, Slot **map,
                    Machine::status_t & status_out) const
{
    assert(_own);
//    assert(stack_base != 0);
//    assert(length >= 32);
    assert(*this);          // Check we are actually runnable
    return m.run(_code, _data, seg, islot_idx, count, nPre, status_out, maxmap, map);
}

