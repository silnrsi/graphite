/*  GRAPHITE2 LICENSING

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
#include "graphite2/Segment.h"
#include "Code.h"
#include "Machine.h"
#include "Silf.h"
#include "Face.h"
#include "Rule.h"
#include "XmlTraceLog.h"

#include <cstdio>

#ifdef DISABLE_TRACING
#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
#endif


using namespace graphite2;
using namespace vm;

namespace {

inline bool is_return(const instr i) {
    const opcode_t * opmap = Machine::getOpcodeTable();
    const instr pop_ret  = *opmap[POP_RET].impl,
                ret_zero = *opmap[RET_ZERO].impl,
                ret_true = *opmap[RET_TRUE].impl;
    return i == pop_ret || i == ret_zero || i == ret_true;
}

void emit_trace_message(opcode, const byte *const, const opcode_t &);

struct context
{
    context(uint8 ref=0) : codeRef(ref) {flags.changed=false; flags.referenced=false; flags.inserted=false;}
    struct { 
        uint8   changed:1,
                referenced:1,
                inserted:1;
    } flags;
    uint8       codeRef;
};

} // end namespace


class Code::decoder
{
public:
    struct limits;
    struct analysis
    {
        int       slotref;
        context   contexts[256];
        
        analysis() : slotref(0) {};

    };
    
    decoder(const limits & lims, Code &code) throw() 
    : _code(code), _instr_count(code._instr_count), _data_size(code._data_size),
      ip(code._code), dp(code._data), max(lims) {}
    
    bool        load(const byte * bc_begin, const byte * bc_end);
    void        apply_analysis(instr * const code, instr * code_end);
    
private:
    opcode      fetch_opcode(const byte * bc) const;
    void        analyse_opcode(const opcode, const int8 * const dp) throw();
    bool        emit_opcode(opcode opc, const byte* & bc);
    void        valid_upto(const uint16 limit, const uint16 x) const throw();
    void        failure(const status_t s) const throw() { const_cast<Code &>(_code).failure(s); }
    
    Code              & _code;
    size_t            & _instr_count,
                      & _data_size;
    instr             * ip;
    byte              * dp;
    const limits      & max;
    analysis            ac;
};


struct Code::decoder::limits 
{
  const byte * const bytecode;
  const uint16       classes,
                     glyf_attrs,
                     features;
  const byte attrid[gr_slatMax];
};
   


Code::Code(bool constrained, const byte * bytecode_begin, const byte * const bytecode_end, 
           uint8 pre_context, uint16 rule_length, const Silf & silf, const Face & face)
 :  _code(0), _data_size(0), _instr_count(0), _min_slotref(0), _max_slotref(0), _status(loaded),
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
    
    // Allocate code and dat target buffers, these sizes are a worst case 
    // estimate.  Once we know their real sizes the we'll shrink them.
    _code = static_cast<instr *>(malloc((bytecode_end - bytecode_begin)
                                             * sizeof(instr)));
    _data = static_cast<byte *>(malloc((bytecode_end - bytecode_begin)
                                             * sizeof(byte)));
    
    if (!_code || !_data) {
        failure(alloc_failed);
        return;
    }
    
    const decoder::limits lims = {
        bytecode_end, 
        silf.numClasses(),
        face.getGlyphFaceCache()->numAttrs(),
        face.numFeatures(), 
        {1,1,1,1,1,1,1,1, 
         1,1,1,1,1,1,1,-1, 
         1,1,1,1,1,1,1,1, 
         1,1,1,1,1,1,0,0, 
         0,0,0,0,0,0,0,0, 
         0,0,0,0,0,0,0,0, 
         0,0,0,0,0,0,0, silf.numUser()}
    };
    
    decoder dec(lims, *this);
    if(!dec.load(bytecode_begin, bytecode_end))
       return;
    
    // Is this an empty program?
    if (_instr_count == 0)
    {
      release_buffers();
      ::new (this) Code();
      return;
    }
    
    // When we reach the end check we've terminated it correctly
    if (!is_return(_code[_instr_count-1])) {
        failure(missing_return);
        return;
    }

    assert((_constrained && immutable()) || !_constrained);
    dec.apply_analysis(_code, _code + _instr_count);
    
    // Now we know exactly how much code and data the program really needs
    // realloc the buffers to exactly the right size so we don't waste any 
    // memory.
    assert((bytecode_end - bytecode_begin) >= ptrdiff_t(_instr_count));
    assert((bytecode_end - bytecode_begin) >= ptrdiff_t(_data_size));
    _code = static_cast<instr *>(realloc(_code, (_instr_count+1)*sizeof(instr)));
    _data = static_cast<byte *>(realloc(_data, _data_size*sizeof(byte)));
    
    // Make this RET_ZERO, we should never reach this but just in case ...
    _code[_instr_count] = op_to_fn[RET_ZERO].impl[constrained];

    if (!_code)
        failure(alloc_failed);
}

Code::~Code() throw ()
{
    if (_own)
        release_buffers();
}


bool Code::decoder::load(const byte * bc, const byte * bc_end)
{
    while (bc < bc_end)
    {
        const opcode opc = fetch_opcode(bc++);
        if (opc == vm::MAX_OPCODE)
            return false;
        
        analyse_opcode(opc, reinterpret_cast<const int8 *>(bc));
        
        if (!emit_opcode(opc, bc))
            return false;
    }
    
    if (bc > bc_end)
        failure(arguments_exhausted);

    return _code.status() == loaded;
}

// Validation check and fixups.
//

opcode Code::decoder::fetch_opcode(const byte * bc) const
{
    opcode opc = opcode(*bc++);

    // Do some basic sanity checks based on what we know about the opcode
    // and it's arguments.
    switch (opc)
    {
        case NOP :
        case PUSH_BYTE :
        case PUSH_BYTEU :
        case PUSH_SHORT :
        case PUSH_SHORTU :
        case PUSH_LONG :
        case ADD :
        case SUB :
        case MUL :
        case DIV :
        case MIN_ :
        case MAX_ :
        case NEG :
        case TRUNC8 :
        case TRUNC16 :
        case COND :
        case AND :
        case OR :
        case NOT :
        case EQUAL :
        case NOT_EQ :
        case LESS :
        case GTR :
        case LESS_EQ :
        case GTR_EQ :
        case NEXT :
        case NEXT_N :           // runtime checked
        case COPY_NEXT :
            break;
        case PUT_GLYPH_8BIT_OBS :
            valid_upto(max.classes, bc[0]);
            break;
        case PUT_SUBS_8BIT_OBS :
            // slot: dp[0] runtime checked
            valid_upto(max.classes, bc[1]);
            valid_upto(max.classes, bc[2]);
            break;
        case PUT_COPY :         // slot: dp[0] runtime checked
        case INSERT :
        case DELETE :
        case ASSOC :            // argn, [arg*]: dp[0].. checked later
            break;
        case CNTXT_ITEM :
            // slot: dp[0] runtime checked
            if (bc + 2 + bc[1] >= max.bytecode)  failure(jump_past_end);
            break;
        case ATTR_SET :
        case ATTR_ADD :
        case ATTR_SUB :
        case ATTR_SET_SLOT :
            valid_upto(gr_slatMax, bc[0]);
            break;
        case IATTR_SET_SLOT :
            valid_upto(gr_slatMax, bc[0]);
            valid_upto(max.attrid[bc[0]], bc[1]);
            break;
        case PUSH_SLOT_ATTR :
            valid_upto(gr_slatMax, bc[0]);
            // slot: dp[1] runtime checked
            break;
        case PUSH_GLYPH_ATTR_OBS :
            valid_upto(max.glyf_attrs, bc[0]);
            // slot: dp[1] runtime checked
            break;
        case PUSH_GLYPH_METRIC :
            valid_upto(kgmetDescent, bc[0]);
            // slot: dp[1] runtime checked
            // level: dp[2] no check necessary
            break;
        case PUSH_FEAT :
            valid_upto(max.features, bc[0]);
            // slot: dp[1] runtime checked
            break;
        case PUSH_ATT_TO_GATTR_OBS :
            valid_upto(max.glyf_attrs, bc[0]);
            // slot: dp[1] runtime checked
            break;
        case PUSH_ATT_TO_GLYPH_METRIC :
            valid_upto(kgmetDescent, bc[0]);
            // slot: dp[1] runtime checked
            // level: dp[2] no check necessary
            break;
        case PUSH_ISLOT_ATTR :
            valid_upto(gr_slatMax, bc[0]);
            // slot: dp[1] runtime checked
            valid_upto(max.attrid[bc[0]], bc[2]);
            break;
        case PUSH_IGLYPH_ATTR :// not implemented
        case POP_RET :
        case RET_ZERO :
        case RET_TRUE :
            break;
        case IATTR_SET :
        case IATTR_ADD :
        case IATTR_SUB :
            valid_upto(gr_slatMax, bc[0]);
            valid_upto(max.attrid[bc[0]], bc[1]);
            break;
        case PUSH_PROC_STATE :  // dummy: dp[0] no check necessary
        case PUSH_VERSION :
            break;
        case PUT_SUBS :
            // slot: dp[0] runtime checked
            valid_upto(max.classes, uint16(bc[1]<< 8) | bc[2]);
            valid_upto(max.classes, uint16(bc[3]<< 8) | bc[4]);
            break;
        case PUT_SUBS2 :        // not implemented
        case PUT_SUBS3 :        // not implemented
            break;
        case PUT_GLYPH :
            valid_upto(max.classes, uint16(bc[0]<< 8) | bc[1]);
            break;
        case PUSH_GLYPH_ATTR :
        case PUSH_ATT_TO_GLYPH_ATTR :
            valid_upto(max.glyf_attrs, uint16(bc[0]<< 8) | bc[1]);
            // slot: dp[2] runtime checked
            break;
        default:
            failure(invalid_opcode);
    }

    return _code._status == loaded ? opc : MAX_OPCODE;
}


void Code::decoder::analyse_opcode(const opcode opc, const int8  * arg) throw()
{
  if (_code._constrained) return;
  
  switch (opc)
  {
    case DELETE :
      _code._delete = true;
      break;
    case PUT_GLYPH_8BIT_OBS :
    case PUT_GLYPH :
      _code._modify = true;
      break;
    case NEXT :
    case COPY_NEXT :
      if (!ac.contexts[ac.slotref].flags.inserted)
        ++ac.slotref;
      ac.contexts[ac.slotref] = context(_instr_count+1);
      break;
    case INSERT :
      ac.contexts[ac.slotref].flags.inserted = true;
      _code._modify = true;
      break;
    case PUT_SUBS_8BIT_OBS :    // slotref on 1st parameter
    case PUT_SUBS : 
      _code._modify = true;
      ac.contexts[ac.slotref].flags.changed = true;
    case PUT_COPY :
    {
      context & ctxt = ac.contexts[ac.slotref];
      if (arg[0] != 0) { ctxt.flags.changed = true; _code._modify = true; }
      if (arg[0] <= 0 && -arg[0] <= ac.slotref)
        ac.contexts[ac.slotref + arg[0] - ctxt.flags.inserted].flags.referenced = true;
      break;
    }
    case PUSH_SLOT_ATTR :       // slotref on 2nd parameter
    case PUSH_GLYPH_ATTR_OBS :
    case PUSH_GLYPH_ATTR :
    case PUSH_ISLOT_ATTR :
      if (arg[1] <= 0 && -arg[1] <= ac.slotref)
        ac.contexts[ac.slotref + arg[1] - ac.contexts[ac.slotref].flags.inserted].flags.referenced = true;
      break;
    case PUSH_GLYPH_METRIC :
    case PUSH_FEAT :
    case PUSH_ATT_TO_GATTR_OBS :
    case PUSH_ATT_TO_GLYPH_METRIC :
    case PUSH_ATT_TO_GLYPH_ATTR :
    case ASSOC :                // slotrefs in varargs
      break;
    default:
        break;
  }
}


bool Code::decoder::emit_opcode(opcode opc, const byte * & bc)
{
    const opcode_t * op_to_fn = Machine::getOpcodeTable();
    const opcode_t & op       = op_to_fn[opc];
    if (op.impl[_code._constrained] == 0)
    {
        failure(unimplemented_opcode_used);
        return false;
    }

    const size_t     param_sz = op.param_sz == VARARGS ? bc[0] + 1 : op.param_sz;

    // Add this instruction
    *ip++ = op.impl[_code._constrained]; 
    ++_instr_count;
    emit_trace_message(opc, bc, op);

    // Grab the parameters
    if (param_sz) {
        memmove(dp, bc, param_sz * sizeof(byte));
        bc         += param_sz;
        dp         += param_sz;
        _data_size += param_sz;
    }
    
    // recursively decode a context item so we can split the skip into 
    // instruction and data portions.
    if (opc == CNTXT_ITEM)
    {
        const size_t ctxt_start = _instr_count;
        byte & instr_skip = dp[-1];
        byte & data_skip  = *dp++;
        ++_data_size;

        load(bc, bc + instr_skip);
        bc += instr_skip;
        data_skip  = instr_skip - (_instr_count - ctxt_start);
        instr_skip = _instr_count - ctxt_start;
    }
    
    return _code._status == loaded;
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

} // end of namespace


void Code::decoder::apply_analysis(instr * const code, instr * code_end)
{
    // insert TEMP_COPY commands for slots that need them (that change and are referenced later)
    if (_code._constrained) return;

    const instr temp_copy = Machine::getOpcodeTable()[TEMP_COPY].impl[0];
    for (const context * c = ac.contexts, * const ce = c + ac.slotref; c != ce; ++c)
    {
        if (!c->flags.referenced || !c->flags.changed) continue;
        
        instr * const tip = code + c->codeRef;        
        memmove(tip+1, tip, (code_end - tip) * sizeof(instr));
        *tip = temp_copy;
        ++code_end;    
    }
    
    _instr_count = code_end - code;
}

inline 
void Code::decoder::valid_upto(const uint16 limit, const uint16 x) const throw()
{
    if (x >= limit)
        failure(out_of_range_data);
}

inline 
void Code::failure(const status_t s) throw() {
    release_buffers();
    _status = s;
}

void Code::release_buffers() throw() 
{
    free(_code);
    free(_data);
    _code = 0;
    _data = 0;
    _own  = false;
}


int32 Code::run(Machine & m, slotref * & map, Machine::status_t & status_out) const
{
    assert(_own);
    assert(*this);          // Check we are actually runnable
    
    if (map + _min_slotref < m.slotMap().begin() 
     || map + _max_slotref >= m.slotMap().end())
    {
      status_out = Machine::slot_offset_out_bounds;
      return 0;
    }
    return  m.run(_code, _data, map, status_out);
}
