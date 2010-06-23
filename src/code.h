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
        empty,
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
    mutable bool _own;

    void release_buffers() throw ();
    void failure(const status_t) throw();

public:
    code() throw();
    code(bool constrained, const byte * bytecode_begin, const byte * const bytecode_end, byte *cConstraints);
    code(const code &) throw();
    ~code() throw();
    
    code & operator=(const code &rhs) throw();
    operator bool () throw();
    status_t    status() const throw();
    size_t      data_size() const throw();
    size_t      instruction_count() const throw();
    
    uint32 run(uint32 * stack_base, const size_t length,
                    Segment & seg, int & islot_idx,
                    machine::status_t & status);
};

inline code::code() throw()
: _code(0), _data(0), _data_size(0), _instr_count(0), 
  _status(empty), _own(false) {
}

inline code::code(const code &obj) throw ()
:_code(obj._code), _data(obj._data), 
 _data_size(obj._data_size), _instr_count(obj._instr_count), 
 _status(obj._status), _own(obj._own) {
    obj._own = false;
}

inline code & code::operator=(const code &rhs) throw() {
    if (_status != empty)   release_buffers();
    _code = rhs._code; _data = rhs._data;
    _data_size = rhs._data_size; _instr_count = rhs._instr_count;
    _status = rhs._status; _own = rhs._own; rhs._own = false;
    return *this;
}

inline code::operator bool () throw () {
    return _code && status() == loaded;
}

inline code::status_t code::status() const throw() {
    return _status;
}

inline size_t code::data_size() const throw() {
    return _data_size;
}

inline size_t code::instruction_count() const throw() {
    return _instr_count;
}

