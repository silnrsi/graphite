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
    byte  * _data;
    size_t  _data_size, 
            _instr_count;
    
    code(const code &);
    void release_buffers() throw ();
    
public:
    code(bool constrained, const byte * bytecode_begin, const byte * const bytecode_end);
    virtual ~code() throw();
    
    size_t data_size() const throw();
    size_t instruction_count() const throw();
    
    uint32 run(uint32 * stack_base, const size_t length,
                    Segment & seg, const int islot_idx);
};

inline size_t code::data_size() const throw() {
    return _data_size;
}

inline size_t code::instruction_count() const throw() {
    return _instr_count;
}

