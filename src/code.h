// This class represents loaded graphite stack machine code.  It performs 
// basic sanity checks, on the incoming code to prevent more obvious problems
// from crashing graphite.
// Author: Tim Eves

#pragma once

#include <graphiteng/Types.h>
#include "machine.h"

class code 
{
    instr * _code;
    size_t  _size, _instr_count;
public:
    code(bool constrained, const byte * bytecode_begin, const byte * const bytecode_end);
    virtual ~code() throw();
    
    size_t size() const throw();
    size_t instruction_count() const throw();
    
    uint32 run(uint32 * stack_base, const size_t length,
                    Segment & seg, const int islot_idx);
};

inline size_t code::size() const throw() {
    return _size;
}

inline size_t code::instruction_count() const throw() {
    return _instr_count;
}

