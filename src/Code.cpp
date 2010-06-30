// This class represents loaded graphite stack machine code.  It performs 
// basic sanity checks, on the incoming code to prevent more obvious problems
// from crashing graphite.
// Author: Tim Eves

#include <cassert>
#include <cstdlib>
#include <stdexcept>
#include "Code.h"
#include "Machine.h"
#include "XmlTraceLog.h"

#include <cstdio>

using namespace vm;

namespace {

inline bool is_return(const opcode opc) {
    return opc == POP_RET || opc == RET_ZERO || opc == RET_TRUE;
}

void emit_trace_message(opcode, const byte *const, const opcode_t &);
void fixup_cntxt_item_target(const byte*, byte * &);

} // end namespace


Code::Code(bool constrained, const byte * bytecode_begin, const byte * const bytecode_end, byte *cContexts)
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
    cContexts[0] = 0;
    cContexts[1] = 0;
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
            fixup_instruction_offsets(opc, reinterpret_cast<int8 *>(dp), param_sz, 
                                      iSlot, cContexts);
    } while (!is_return(opc) && cd_ptr < bytecode_end);
    
    // Final sanity check: ensure that the program is correctly terminated.
    if (!is_return(opc)) {
        failure(missing_return);
        return;
    }
    
    assert(ip - _code == ptrdiff_t(_instr_count));
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

#define ctxtins(n)  cContexts[(n)*2]
#define ctxtdel(n)  cContexts[(n)*2+1]
inline void fixup_slotref(int8 * const arg, uint8 is, const byte *const cContexts) {
    if (*arg < 0)
        *arg -= ctxtins(is + *arg);
//    else
//        *arg += ctxtins(is);
}

} // end of namespace

void Code::fixup_instruction_offsets(const opcode opc, 
                                     int8  * dp, size_t param_sz,
                                     byte & iSlot, byte * cContexts)
{
    
    uint8 *contexts = static_cast<uint8 *>(cContexts);
    
    switch (opc)
    {
        case NEXT :
        case COPY_NEXT :
            iSlot++;
            ctxtins(iSlot) = 0;
            ctxtdel(iSlot) = 0;
            break;
        case INSERT :
            for (int i = iSlot; i >= 0; --i)
                ++ctxtins(i);
            break;
        case DELETE :
            for (int i = iSlot; i >= 0; --i)
                ++ctxtdel(i);
            break;
        case PUT_COPY :
        case PUSH_SLOT_ATTR :
        case PUSH_GLYPH_ATTR_OBS :
        case PUSH_FEAT :
        case PUSH_ATT_TO_GATTR_OBS :
        case PUSH_GLYPH_ATTR :
        case PUSH_ATT_TO_GLYPH_ATTR :
            fixup_slotref(dp-1,iSlot,cContexts);
            break;
        case PUSH_GLYPH_METRIC :
        case PUSH_ATT_TO_GLYPH_METRIC :
        case PUSH_ISLOT_ATTR :
            fixup_slotref(dp-2,iSlot,cContexts);
            break;
        case CNTXT_ITEM :
        case PUT_SUBS_8BIT_OBS:
            fixup_slotref(dp-3,iSlot,cContexts);
	        break;
        case PUT_SUBS :
            fixup_slotref(dp-5,iSlot,cContexts);
            break;
        case ASSOC :
            for (size_t i = 1; i < param_sz; ++i)
                fixup_slotref(dp-i,iSlot,cContexts);
            break;
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


int32 Code::run(Machine & m, Segment & seg, slotref & islot_idx,
                    Machine::status_t & status_out) const
{
    assert(_own);
//    assert(stack_base != 0);
//    assert(length >= 32);
    assert(*this);          // Check we are actually runnable
    return m.run(_code, _data, seg, islot_idx, status_out);
}

