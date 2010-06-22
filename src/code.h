// This class represents loaded graphite stack machine code.  It performs 
// basic sanity checks, on the incoming code to prevent more obvious problems
// from crashing graphite.
// Author: Tim Eves

#pragma once

#include <graphiteng/Types.h>
#include "machine.h"

class code 
{
public:
    enum status_t 
    {
        loaded,
        alloc_failed, 
        invalid_opcode, 
        unimplemented_opcode_used,
        jump_past_end,
        arguments_exhausted,
        missing_return
    };

private:
    instr *     _code;
    byte  *     _data;
    size_t      _data_size, 
                _instr_count;
    status_t    _status;

    code(const code &);
    void release_buffers() throw ();
    void failure(const status_t) throw();

public:
    
    code(bool constrained, const byte * bytecode_begin, const byte * const bytecode_end);
    ~code() throw();
    
    operator bool () throw();
    status_t status() throw();
    size_t data_size() const throw();
    size_t instruction_count() const throw();
    
    uint32 run(uint32 * stack_base, const size_t length,
                    Segment & seg, int & islot_idx,
                    machine::status_t & status);
};


inline code::operator bool () throw () {
    return bool(status() == loaded);
}

inline code::status_t code::status() throw() {
    return _status;
}

inline size_t code::data_size() const throw() {
    return _data_size;
}

inline size_t code::instruction_count() const throw() {
    return _instr_count;
}

