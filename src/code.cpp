// This class represents loaded graphite stack machine code.  It performs 
// basic sanity checks, on the incoming code to prevent more obvious problems
// from crashing graphite.
// Author: Tim Eves

#include <cassert>
#include <cstdlib>
#include <stdexcept>
#include "code.h"
#include "machine.h"


code::code(bool constrained, const byte * bytecode_begin, const byte * const bytecode_end, byte *cContexts)
: _code(0), _data_size(0), _instr_count(0), _status(loaded), _own(true)
{
    assert(bytecode_begin != 0);
    assert(bytecode_end > bytecode_begin);
    
    const opcode_t *    op_to_fn = machine::get_opcode_table(constrained);
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
    
    instr * ip = _code;
    byte  * dp = _data;
    cContexts[0] = 0;
    cContexts[1] = 0;
    do {
        const machine::opcode opc = machine::opcode(*cd_ptr++);
        
        // Do some basic sanity checks based on what we know about the opcodes.
        if (opc >= machine::MAX_OPCODE) {   // Is this even a valid opcode?
            failure(invalid_opcode);
            return;
        }

        const opcode_t op = op_to_fn[opc];
        if (op.param_sz == NILOP) {      // Is it implemented?
            failure(unimplemented_opcode_used);
            return;
        }

        if (opc == machine::CNTXT_ITEM)  // This is a really conditional forward jump,
        {                       // check it doesn't jump outside the program.
            const size_t skip = cd_ptr[1];
            if (cd_ptr + 2 + skip > bytecode_end) {
                failure(jump_past_end);
                return;
            }
        }
        
        const size_t param_sz = op.param_sz == VARARGS ? *cd_ptr++ : op.param_sz;
        if (cd_ptr + param_sz > bytecode_end) { // Is the requested size possible
            failure(arguments_exhausted);
            return;
        }
        
        // Add this instruction
        *ip++ = op.impl; ++_instr_count;
        // Grab the parameters
        if (param_sz)
        {
            std::copy(cd_ptr, cd_ptr + param_sz, dp);
            cd_ptr += param_sz;
            dp     += param_sz;
        }

        switch (opc)
        {
            case machine::NEXT :
            case machine::COPY_NEXT :
                iSlot++;
                cContexts[iSlot * 2] = 0;
                cContexts[iSlot * 2 + 1] = 0;
                break;
            case machine::INSERT :
                for (char i = iSlot; i >= 0; i--)
                    cContexts[i * 2]++;
                break;
            case machine::DELETE :
                for (char i = iSlot; i >= 0; i--)
                    cContexts[i * 2 + 1]++;
                break;
            case machine::PUT_COPY :
            case machine::PUSH_SLOT_ATTR :
            case machine::PUSH_GLYPH_ATTR :
            case machine::PUSH_ATT_TOG_ATTR_OBS :
                if (dp[-1] > 128)
                    dp[-1] -= cContexts[2 * (iSlot + 256 - dp[-1])];
                break;
            case machine::CNTXT_ITEM :
            case machine::PUSH_ISLOT_ATTR :
            case machine::PUSH_GLYPH_METRIC :
            case machine::PUSH_ATT_TOGLYPH_METRIC :
                if (dp[-2] > 128)
                    dp[-2] -= cContexts[2 * (iSlot + 256 - dp[-2])];
                break;
            case machine::PUT_SUBS :
                if (dp[-3] > 128)
                    dp[-3] -= cContexts[2 * (iSlot + 256 - dp[-3])];
                break;
            case machine::ASSOC :
                for (byte i = 0; i < param_sz; i++)
                    if (dp[-i] > 128)
                        dp[-i] -= cContexts[2 * (iSlot + 256 - dp[-i])];
                break;
            default :
                break;
        }

        // Was this a return? stop processing any further.
        if (opc == machine::POP_RET
         || opc == machine::RET_ZERO 
         || opc == machine::RET_TRUE)
            break;
    } while (cd_ptr < bytecode_end);
    
    // Final sanity check: ensure that the program is correctly terminated.
    switch (*(cd_ptr-1)) {
        case machine::POP_RET: 
        case machine::RET_ZERO: 
        case machine::RET_TRUE: 
            break;
        default:
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

