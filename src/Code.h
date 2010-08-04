// This class represents loaded graphite stack machine code.  It performs 
// basic sanity checks, on the incoming code to prevent more obvious problems
// from crashing graphite.
// Author: Tim Eves

#pragma once

#include <graphiteng/Types.h>
#include "Main.h"
#include "Machine.h"

using namespace org::sil::graphite::v2;

namespace vm
{

struct CodeContext
{
    CodeContext(uint8 ins, uint8 copy, uint8 ref)
        : nInserts(ins), copySlot(copy), codeRef(ref) {}
    CodeContext() : nInserts(0), copySlot(0), codeRef(0) {}
    uint8       nInserts;
    uint8       copySlot;
    uint8       codeRef;
};

class Code 
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
    bool        _constrained;
    mutable bool _own;

    void release_buffers() throw ();
    void failure(const status_t) throw();
    bool check_opcode(const opcode, const byte *, const byte *const);
    void fixup_instruction_offsets(const opcode, size_t, int8  *, size_t, byte &, CodeContext *);
public:
    Code() throw();
    Code(bool constrained, const byte* bytecode_begin, const byte* const bytecode_end, CodeContext* cContexts);
    Code(const Code &) throw();
    ~Code() throw();
    
    Code & operator=(const Code &rhs) throw();
    operator bool () const throw();
    status_t    status() const throw();
    bool        constraint() const throw();
    size_t      dataSize() const throw();
    size_t      instructionCount() const throw();
    
    int32 run(Machine &m, GrSegment & seg, slotref & islot_idx, slotref iStart,
                    Machine::status_t & status) const;
    CLASS_NEW_DELETE
};

inline Code::Code() throw()
: _code(0), _data(0), _data_size(0), _instr_count(0), 
  _status(empty), _own(false) {
}

inline Code::Code(const Code &obj) throw ()
 :  _code(obj._code), 
    _data(obj._data), 
    _data_size(obj._data_size), 
    _instr_count(obj._instr_count), 
    _status(obj._status), 
    _constrained(obj._constrained), 
    _own(obj._own) 
{
    obj._own = false;
}

inline Code & Code::operator=(const Code &rhs) throw() {
    if (_status != empty)
        release_buffers();
    _code        = rhs._code; 
    _data        = rhs._data;
    _data_size   = rhs._data_size; 
    _instr_count = rhs._instr_count;
    _status      = rhs._status; 
    _constrained = rhs._constrained;
    _own         = rhs._own; 
    rhs._own = false;
    return *this;
}

inline Code::operator bool () const throw () {
    return _code && status() == loaded;
}

inline Code::status_t Code::status() const throw() {
    return _status;
}

inline bool Code::constraint() const throw() {
    return _constrained;
}

inline size_t Code::dataSize() const throw() {
    return _data_size;
}

inline size_t Code::instructionCount() const throw() {
    return _instr_count;
}



} // end of namespace vm

