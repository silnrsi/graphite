/*  GRAPHITENG LICENSING

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

#pragma once

#include <utility>
#include <graphite2/Types.h>
#include "Main.h"
#include "Machine.h"


namespace vm
{

class Code 
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
    struct Context
    {
        Context(uint8 ref=0) : codeRef(ref) {flags.changed=false; flags.referenced=false; flags.inserted=false;}
        struct { 
          uint8 changed:1,
                referenced:1,
                inserted:1;
        } flags;
        uint8       codeRef;
    };

    struct analysis_context
    {
      int       slotref;
      Context   contexts[256];
      
      analysis_context();
    };
    
    instr *     _code;
    byte  *     _data;
    size_t      _data_size,
                _instr_count;
    int         _min_slotref,
                _max_slotref;
    mutable status_t _status;
    bool        _constrained,
                _modify,
                _delete;
    mutable bool _own;

    void release_buffers() throw ();
    void failure(const status_t) throw();
    bool check_opcode(const opcode, const byte *, const byte *const);
    void analyse_opcode(const opcode, size_t cp, const int8  * dp, analysis_context &) throw();
    void update_slot_limits(int slotref) throw ();
public:
    Code() throw();
    Code(bool constrained, const byte* bytecode_begin, const byte* const bytecode_end);
    Code(const Code &) throw();
    ~Code() throw();
    
    Code & operator=(const Code &rhs) throw();
    operator bool () const throw();
    status_t      status() const throw();
    bool          constraint() const throw();
    size_t        dataSize() const throw();
    size_t        instructionCount() const throw();
    bool          immutable() const throw();
    bool          deletes() const throw();

    int32 run(Machine &m, slotref * & map, Machine::status_t & status) const;
    CLASS_NEW_DELETE
};

inline Code::Code() throw()
: _code(0), _data(0), _data_size(0), _instr_count(0), _min_slotref(0), _max_slotref(0),
  _status(loaded), _own(false) {
}

inline Code::Code(const Code &obj) throw ()
 :  _code(obj._code), 
    _data(obj._data), 
    _data_size(obj._data_size), 
    _instr_count(obj._instr_count),
    _min_slotref(obj._min_slotref),
    _max_slotref(obj._max_slotref),
    _status(obj._status), 
    _constrained(obj._constrained),
    _modify(obj._modify),
    _delete(obj._delete),
    _own(obj._own) 
{
    obj._own = false;
}

inline Code & Code::operator=(const Code &rhs) throw() {
    if (_instr_count > 0)
        release_buffers();
    _code        = rhs._code; 
    _data        = rhs._data;
    _data_size   = rhs._data_size; 
    _instr_count = rhs._instr_count;
    _min_slotref = rhs._min_slotref;
    _max_slotref = rhs._max_slotref;
    _status      = rhs._status; 
    _constrained = rhs._constrained;
    _modify      = rhs._modify;
    _delete      = rhs._delete;
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

inline bool Code::immutable() const throw()
{
  return !(_delete || _modify);
}

inline bool Code::deletes() const throw()
{
  return _delete;
}

} // end of namespace vm

