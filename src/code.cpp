// This class represents loaded graphite stack machine code.  It performs 
// basic sanity checks, on the incoming code to prevent more obvious problems
// from crashing graphite.
// Author: Tim Eves

#include <cassert>
#include <cstdlib>
#include <stdexcept>
#include "code.h"
#include "machine.h"

namespace 
{

inline bool is_return(const machine::opcode opc) {
    using namespace machine;
    return opc == POP_RET || opc == RET_ZERO || opc == RET_TRUE;
}

}


code::code(bool constrained, const byte * bytecode_begin, const byte * const bytecode_end, byte *cContexts)
: _code(0), _data_size(0), _instr_count(0), _status(loaded), _own(true)
{
    assert(bytecode_begin != 0);
    assert(bytecode_end > bytecode_begin);
    
    const opcode_t *    op_to_fn = machine::get_opcode_table();
    const byte *        cd_ptr = bytecode_begin;
    byte                iSlot = 0;
    
    // Allocate code and dat target buffers, these sizes are a worst case 
    // estimate.  Once we know their real sizes the we'll shrink them.
    _code = static_cast<instr *>(std::malloc((bytecode_end - bytecode_begin)
                                             * sizeof(instr)));
    _data = static_cast<byte *>(std::malloc((bytecode_end - bytecode_begin)));
    
    if (!_code || !_data) {
        failure(alloc_failed);
        return;
    }
    
    instr *         ip = _code;
    byte  *         dp = _data;
    machine::opcode opc;
    cContexts[0] = 0;
    cContexts[1] = 0;
    do {
        opc = machine::opcode(*cd_ptr++);
        
        // Do some basic sanity checks based on what we know about the opcodes.
        if (!check_opcode(opc, cd_ptr, bytecode_end))
            return;

        const opcode_t op = op_to_fn[opc];
        if (op.impl[constrained] == 0) {      // Is it implemented?
            failure(unimplemented_opcode_used);
            return;
        }
        
        const size_t param_sz = op.param_sz == VARARGS ? *cd_ptr++ : op.param_sz;
        if (cd_ptr + param_sz > bytecode_end) { // Is the requested size possible
            failure(arguments_exhausted);
            return;
        }
        
        // Add this instruction
        *ip++ = op.impl[constrained]; 
        ++_instr_count;
        
        // Grab the parameters
        if (param_sz) {
            std::copy(cd_ptr, cd_ptr + param_sz, dp);
            cd_ptr += param_sz;
            dp     += param_sz;
        }
        
        fixup_instruction_offsets(opc, dp, param_sz, iSlot, cContexts);
    } while (!is_return(opc) && cd_ptr < bytecode_end);
    
    // Final sanity check: ensure that the program is correctly terminated.
    if (!is_return(opc)) {
        failure(missing_return);
        return;
    }
    
    assert(ip - _code == _instr_count);
    _data_size = sizeof(byte)*(dp - _data);
    
    // Now we know exactly how much code and data the program really needs
    // realloc the buffers to exactly the right size so we don't waste any 
    // memory.
    _code = static_cast<instr *>(std::realloc(_code, _instr_count*sizeof(instr)));
    _data = static_cast<byte *>(std::realloc(_data, _data_size*sizeof(byte)));
}

code::~code() throw ()
{
    if (_own)
        release_buffers();
}


// Validation check and fixups.
//
bool code::check_opcode(const machine::opcode opc, 
                        const byte *cd_ptr, 
                        const byte *const cd_end) 
{
    using namespace machine;
    
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


void code::fixup_instruction_offsets(const machine::opcode opc, 
                                     byte  * dp, size_t param_sz,
                                     byte & iSlot, byte * cContexts)
{
    using namespace machine;
    
    switch (opc)
    {
        case NEXT :
        case COPY_NEXT :
            iSlot++;
            cContexts[iSlot * 2] = 0;
            cContexts[iSlot * 2 + 1] = 0;
            break;
        case INSERT :
            for (char i = iSlot; i >= 0; i--)
                cContexts[i * 2]++;
            break;
        case DELETE :
            for (char i = iSlot; i >= 0; i--)
                cContexts[i * 2 + 1]++;
            break;
        case PUT_COPY :
        case PUSH_SLOT_ATTR :
        case PUSH_GLYPH_ATTR :
        case PUSH_ATT_TO_GATTR_OBS :
            if (dp[-1] > 128)
                dp[-1] -= cContexts[2 * (iSlot + 256 - dp[-1])];
            break;
        case CNTXT_ITEM :
        case PUSH_ISLOT_ATTR :
        case PUSH_GLYPH_METRIC :
        case PUSH_ATT_TO_GLYPH_METRIC :
            if (dp[-2] > 128)
                dp[-2] -= cContexts[2 * (iSlot + 256 - dp[-2])];
            break;
        case PUT_SUBS :
            if (dp[-3] > 128)
                dp[-3] -= cContexts[2 * (iSlot + 256 - dp[-3])];
            break;
        case ASSOC :
            for (byte i = 1; i < param_sz; i++)
                if (dp[-i] > 128)
                    dp[-i] -= cContexts[2 * (iSlot + 256 - dp[-i])];
            break;
        default :
            break;
    }
}


inline 
void code::failure(const status_t s) throw() {
    release_buffers();
    _status = s;
}

void code::release_buffers() throw() 
{
    std::free(_code);
    std::free(_data);
    _code = 0;
    _data = 0;
    _own  = false;
}


uint32 code::run(uint32 * stack_base, const size_t length,
                    Segment & seg, int & islot_idx, 
                    machine::status_t & status)
{
    assert(stack_base != 0);
    assert(length >= 32);
    assert(*this);          // Check we are actually runnable
    
    return machine::run(_code, _data, stack_base, length, seg, islot_idx, status);
}

