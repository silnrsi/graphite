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
// This direct threaded interpreter implmentation for machine.h
// Author: Tim Eves

// Build either this interpreter or the call_machine implementation.
// The direct threaded interpreter is relies upon a gcc feature called 
// labels-as-values so is only portable to compilers that support the 
// extension (gcc only as far as I know) however it should build on any
// architecture gcc supports. 
// This is twice as fast as the call threaded model and is likely faster on 
// inorder processors with short pipelines and little branch prediction such 
// as the ARM and possibly Atom chips.


#include <cassert>
#include <cstring>
#include "Machine.h"
#include "GrSegmentImp.h"
#include "XmlTraceLog.h"
#include "SlotImp.h"
#include "Rule.h"

#define STARTOP(name)           name: {
#define ENDOP                   }; goto *((sp - sb)/Machine::STACK_MAX ? &&end : *++ip);
#define EXIT(status)            push(status); goto end

#define do_(name)               &&name

using namespace vm;
using namespace org::sil::graphite::v2;

namespace {

const void * direct_run(const bool          __get_table_mode=true,
                        const instr       * __program=0,
                        const byte        * __data=0,
                        Machine::stack_t  * __stack=0,
                        slotref           * __slot=0,
                        int               * __count=0,
                        gr2::SlotMap      * __map=0)
{
    // We need to define and return to opcode table from within this function 
    // other inorder to take the addresses of the instruction bodies.
    #include "opcode_table.h"
    if (__get_table_mode)
        return opcode_table;

    // Declare virtual machine registers
    const instr       * ip = __program;
    const byte        * dp = __data;
    Machine::stack_t  * sp = __stack + Machine::STACK_GUARD,
                * const sb = sp;
    gr2::SlotMap     & map = *__map;
    GrSegment        & seg = map.segment;
    slotref             is = *__slot;
    int              count = *__count;
    int8             flags = 0;
    
    // start the program
    goto **ip;

    // Pull in the opcode definitions
    #include "opcodes.h"
    
    end:
    *__slot  = is;
    *__count = count;
    return sp;
}

}

const opcode_t * Machine::getOpcodeTable() throw()
{
    return static_cast<const opcode_t *>(direct_run());
}


Machine::stack_t  Machine::run(const instr   * program,
                               const byte    * data,
                               slotref       & slot,
                               int           & count,
                               status_t      & status)
{
    assert(program != 0);
    
    const stack_t *sp = static_cast<const stack_t *>(
                direct_run(false, program, data, _stack, &slot, &count, &_map));
    const stack_t ret = sp == _stack+STACK_GUARD+1 ? *sp-- : 0;
    check_final_stack(sp, status);
    return ret;
}

