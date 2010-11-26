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

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <stdexcept>
#include <string.h>
#include "Code.h"
#include "Machine.h"
#include "Rule.h"
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


Code::analysis_context::analysis_context()
: slotref(0)
{
//   contexts[0] = Context();
}


Code::Code(bool constrained, const byte * bytecode_begin, const byte * const bytecode_end)
 :  _code(0), _data_size(0), _instr_count(0), _status(loaded), _min_slotref(0), _max_slotref(0),
    _constrained(constrained), _modify(false), _delete(false), _own(true)
{
    assert(bytecode_begin != 0);
    if (bytecode_begin == bytecode_end)
    {
      ::new (this) Code();
      return;
    }
    assert(bytecode_end > bytecode_begin);
    const opcode_t *    op_to_fn = Machine::getOpcodeTable();
    const byte *        cd_ptr = bytecode_begin;
    
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
    analysis_context ac;
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
        
        // Analyise the opcode.
        analyse_opcode(opc, _instr_count+1, reinterpret_cast<const int8 *>(cd_ptr), param_sz, ac);
        
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
    } while (!is_return(opc) && cd_ptr < bytecode_end);
    
    // Final sanity check: ensure that the program is correctly terminated.
    if (!is_return(opc)) {
        failure(missing_return);
        return;
    }
    
    assert((_constrained && immutable()) || !_constrained);
    assert(ip - _code == ptrdiff_t(_instr_count));

    // insert TEMP_COPY commands for slots that need them (that change and are referenced later)
    if (!constrained)
    {
      for (const Context * c = ac.contexts, * const ce = c + ac.slotref; c != ce; ++c)
      {
        if (!c->flags.referenced || !c->flags.changed) continue;
        
        instr * const tip = _code + c->codeRef;        
        std::copy_backward(tip, ip, ip+1);
        *tip = op_to_fn[TEMP_COPY].impl[constrained];
        ++_instr_count; ++ip;       
      }
    }

    assert(ip - _code == ptrdiff_t(_instr_count));
    _data_size = sizeof(byte)*(dp - _data);
    
    // Now we know exactly how much code and data the program really needs
    // realloc the buffers to exactly the right size so we don't waste any 
    // memory.
    assert((bytecode_end - bytecode_begin) >= ptrdiff_t(_instr_count));
    assert((bytecode_end - bytecode_begin) >= ptrdiff_t(_data_size));
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
        if (cd_ptr + 2 + skip >= cd_end) {
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
        data_skip += param_sz + (opc == CNTXT_ITEM);
        count     -= 1 + param_sz;
    }
    assert(count == 0);
    dp[-1] -= data_skip;
    *dp++   = data_skip;
}

} // end of namespace

void Code::analyse_opcode(const opcode opc, size_t op_idx,
                           const int8  * dp, size_t param_sz,
                           analysis_context & ab)
{
  if (_constrained) return;
  
  switch (opc)
  {
//     case NOP :                  // no slot ref
//     case PUSH_BYTE : case PUSH_BYTE_U :
//     case PUSH_SHORT : case PUSH_SHORT_U :
//     case PUSH_LONG :
//     case ADD :
//     case SUB :
//     case MUL :
//     case DIV :
//     case MIN :
//     case MAX :
//     case NEG :
//     case TRUNC8 : case TRUNC16 :
//     case COND :
//     case AND :
//     case OR :
//     case NOT :
//     case EQUAL :
//     case NOT_EQ :
//     case LESS :
//     case GTR :
//     case LESS_EQ :
//     case GTR_EQ :
    case DELETE :
    case TEMP_COPY :
      _delete = true;
      break;
    case PUT_GLYPH_8BIT_OBS :
    case PUT_GLYPH :
      _modify = true;
      break;
//     case CNTXT_ITEM :
//     case ATTR_SET :
//     case ATTR_ADD :
//     case ATTR_SUB :
//     case ATTR_SET_SLOT :
//     case IATTR_SET_SLOT :
//     case POP_RET : 
//     case RET_ZERO :
//     case RET_TRUE :
//     case IATTR_SET :
//     case IATTR_ADD :
//     case IATTR_SUB :
//     case PUSH_PROC_STATE :
//     case PUSH_VERSION :

    case NEXT :
    case COPY_NEXT :
      if (!ab.contexts[ab.slotref].flags.inserted)
        ++ab.slotref;
      ab.contexts[ab.slotref] = Context(op_idx);
      break;
      
    case INSERT :
      ab.contexts[ab.slotref].flags.inserted = true;
      _modify = true;
      break;

    case PUT_SUBS_8BIT_OBS :    // slotref on 1st parameter
    case PUT_SUBS : 
      _modify = true;
      ab.contexts[ab.slotref].flags.changed = true;
    case PUT_COPY :
    {
      update_slot_limits(ab.slotref + dp[0]);

      Context & ctxt = ab.contexts[ab.slotref];
      if (dp[0] != 0) ctxt.flags.changed = true;
      if (dp[0] <= 0 && -dp[0] <= ab.slotref)
        ab.contexts[ab.slotref + dp[0] - ctxt.flags.inserted].flags.referenced = true;
      break;
    }
    case PUSH_SLOT_ATTR :       // slotref on 2nd parameter
    case PUSH_GLYPH_ATTR_OBS :
    case PUSH_GLYPH_ATTR :
    case PUSH_ISLOT_ATTR :
      if (dp[1] <= 0 && -dp[1] <= ab.slotref)
        ab.contexts[ab.slotref + dp[1] - ab.contexts[ab.slotref].flags.inserted].flags.referenced = true;
    case PUSH_GLYPH_METRIC :
    case PUSH_FEAT :
    case PUSH_ATT_TO_GATTR_OBS :
    case PUSH_ATT_TO_GLYPH_METRIC :
    case PUSH_ATT_TO_GLYPH_ATTR :
      update_slot_limits(ab.slotref + dp[1]);
      break;
      
    case ASSOC :                // slotrefs in varargs
      uint8 num = *dp+1;
      while (--num) update_slot_limits(ab.slotref + *++dp);
      break;
  }
}


inline
void Code::update_slot_limits(int slotref) throw() {
  _min_slotref = std::min(_min_slotref, slotref);
  _max_slotref = std::max(_max_slotref, slotref);
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


int32 Code::run(Machine & m, slotref & islot_idx, int &count,
                    Machine::status_t & status_out) const
{
    assert(_own);
    assert(*this);          // Check we are actually runnable
    assert(islot_idx == m.slotMap()[count]);
    
    if (count + _min_slotref < 0 || size_t(count + _max_slotref) >= m.slotMap().size())
    {
      status_out = Machine::slot_offset_out_bounds;
      return 0;
    }
    return m.run(_code, _data, islot_idx, count, status_out);
}
