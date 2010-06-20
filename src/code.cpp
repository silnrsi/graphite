// This class represents loaded graphite stack machine code.  It performs 
// basic sanity checks, on the incoming code to prevent more obvious problems
// from crashing graphite.
// Author: Tim Eves

#include <cassert>
#include <cstdlib>
#include <stdexcept>
#include "code.h"
#include "machine.h"


code::code(bool constrained, const byte * bytecode_begin, const byte * const bytecode_end)
: _code(0), _data_size(0), _instr_count(0)
{
    assert(bytecode_begin != 0);
    assert(bytecode_end > bytecode_begin);
    
    const opcode_t *    op_to_fn = machine::get_opcode_table(constrained);
    const byte *        cd_ptr = bytecode_begin;
    
    // Allocate code and dat target buffers, these sizes are a worst case 
    // estimate.  Once we know their real sizes the we'll shrink them.
    _code = static_cast<instr *>(std::malloc((bytecode_end - bytecode_begin)
                                             * sizeof(instr)));
    _data = static_cast<byte *>(std::malloc((bytecode_end - bytecode_begin)));
    instr * ip = _code;
    byte  * dp = _data;
    
    do {
        const machine::opcode opc = machine::opcode(*cd_ptr++);
        
        // Do some basic sanity checks based on what we know about the opcodes.
        if (opc >= machine::MAX_OPCODE) {   // Is this even a valid opcode?
            release_buffers();;
            throw std::range_error("invalid opcode");
        }

        const opcode_t op = op_to_fn[opc];
        if (op.param_sz == NILOP) {      // Is it implemented?
            release_buffers();;
            throw std::runtime_error("attempt to execute unimplemented opcode");
        }

        if (opc == machine::CNTXT_ITEM)  // This is a really conditional forward jump,
        {                       // check it doesn't jump outside the program.
            const size_t skip = cd_ptr[1];
            if (cd_ptr + 2 + skip > bytecode_end) {
                release_buffers();;
                throw std::runtime_error("cntxt_item: jump past end of program");
            }
        }
        
        const size_t param_sz = op.param_sz == VARARGS ? *cd_ptr++ : op.param_sz;
        if (cd_ptr + param_sz > bytecode_end) { // Is the requested size possible
            release_buffers();;
            throw std::runtime_error("arguments exhausted");
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
            release_buffers();
            throw std::runtime_error("No return instruction found");
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
    release_buffers();
}


void code::release_buffers() throw() 
{
    std::free(_code);
    std::free(_data);
}


uint32 code::run(uint32 * stack_base, const size_t length,
                    Segment & seg, const int islot_idx)
{
    assert(stack_base != 0);
    assert(length >= 32);
    
    return machine::run(_code, _data, stack_base, length, seg, islot_idx);
}

