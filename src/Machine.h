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
// This general interpreter interface.
// Author: Tim Eves

// Build one of direct_machine.cpp or call_machine.cpp to implement this 
// interface.

#pragma once
#include <graphite2/Types.h>
#include "Main.h"

#if defined(__GNUC__)
#define     HOT             __attribute__((hot))
#define     REGPARM(n)      __attribute__((hot, regparm(n)))
#else
#define     HOT
#define     REGPARM(n)
#endif

namespace gr2 = org::sil::graphite::v2;

// Forward declarations
namespace org { namespace sil { namespace graphite { namespace v2 {
    struct GrSegment;
    struct GrSlot;
    class SlotMap;
}}}}


namespace vm 
{


typedef void * instr;
typedef gr2::GrSlot * slotref;

enum {VARARGS = size_t(-1), MAX_NAME_LEN=32};

struct opcode_t 
{ 
    instr           impl[2];
    size_t          param_sz;
    int             stack_delta;
    char            name[MAX_NAME_LEN];
};

enum opcode {
    NOP = 0,

    PUSH_BYTE,      PUSH_BYTEU,     PUSH_SHORT,     PUSH_SHORTU,    PUSH_LONG,

    ADD,            SUB,            MUL,            DIV,
    MIN_,           MAX_,
    NEG,
    TRUNC8,         TRUNC16,

    COND,

    AND,            OR,             NOT,
    EQUAL,          NOT_EQ,
    LESS,           GTR,            LESS_EQ,        GTR_EQ,

    NEXT,           NEXT_N,         COPY_NEXT,
    PUT_GLYPH_8BIT_OBS,              PUT_SUBS_8BIT_OBS,   PUT_COPY,
    INSERT,         DELETE,
    ASSOC,
    CNTXT_ITEM,

    ATTR_SET,       ATTR_ADD,       ATTR_SUB,
    ATTR_SET_SLOT,
    IATTR_SET_SLOT,
    PUSH_SLOT_ATTR,                 PUSH_GLYPH_ATTR_OBS,
    PUSH_GLYPH_METRIC,              PUSH_FEAT,
    PUSH_ATT_TO_GATTR_OBS,          PUSH_ATT_TO_GLYPH_METRIC,
    PUSH_ISLOT_ATTR,

    PUSH_IGLYPH_ATTR,    // not implemented

    POP_RET,                        RET_ZERO,           RET_TRUE,
    IATTR_SET,                      IATTR_ADD,          IATTR_SUB,
    PUSH_PROC_STATE,                PUSH_VERSION,
    PUT_SUBS,                       PUT_SUBS2,          PUT_SUBS3,
    PUT_GLYPH,                      PUSH_GLYPH_ATTR,    PUSH_ATT_TO_GLYPH_ATTR,
    MAX_OPCODE,
    // private opcodes for internal use only, comes after all other on disk opcodes
    TEMP_COPY = MAX_OPCODE
};



class Machine
{
public:
    typedef gr2::int32  stack_t;
    static size_t const STACK_ORDER  = 10,
                        STACK_MAX    = 1 << STACK_ORDER,
                        STACK_GUARD  = 2;

    enum status_t {
        finished = 0,
        stack_underflow,
        stack_not_empty,
        stack_overflow,
        slot_offset_out_bounds
    };

    Machine(gr2::SlotMap &) throw();
    static const opcode_t *   getOpcodeTable() throw();
    stack_t                   run(const instr * program, const gr2::byte * data,
                                  slotref * & map,
                                  status_t &status) HOT;
    CLASS_NEW_DELETE

    gr2::SlotMap   & slotMap() const throw();
private:
    void check_final_stack(const stack_t * const sp, status_t &status);

    gr2::SlotMap      & _map;
    stack_t             _stack[STACK_MAX + 2*STACK_GUARD];
};

inline Machine::Machine(gr2::SlotMap & map) throw()
: _map(map)
{
}

inline gr2::SlotMap& Machine::slotMap() const throw()
{
  return _map;
}

inline void Machine::check_final_stack(const gr2::int32 * const sp,
                                       status_t & status) {
    stack_t const * const base  = _stack + STACK_GUARD,
                  * const limit = base + STACK_MAX;
    if      (sp <  base)    status = stack_underflow;       // This should be impossible now.
    else if (sp >= limit)   status = stack_overflow;        // So should this.
    else if (sp != base)    status = stack_not_empty;
    else                    status = finished;
}


} // end of namespace vm

#ifdef ENABLE_DEEP_TRACING
#define STARTTRACE(name,is) if (XmlTraceLog::get().active()) { \
                                XmlTraceLog::get().openElement(ElementOpCode); \
                                XmlTraceLog::get().addAttribute(AttrName, # name); \
                                XmlTraceLog::get().addAttribute(AttrIndex, unsigned(map - smap.begin())); \
                            }

#define ENDTRACE            XmlTraceLog::get().closeElement(ElementOpCode)
#else
#define STARTTRACE(name,is)
#define ENDTRACE
#endif



