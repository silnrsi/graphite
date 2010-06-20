// This call threaded interpreter implmentation for machine.h
// Author: Tim Eves

// Build either this interpreter or the direct_machine implementation.
// The call threaded interpreter is portable across compilers and 
// architectures as well as being useful to debug (you can set breakpoints on 
// opcodes) but is slower that the direct threaded interpreter by a factor of 2

#include <cassert>
#include "machine.h"

#define registers           const byte * & dp, uint32 * & sp, Segment & seg, \
                            uint32 & is, uint32 & os, const instr * & ip
typedef ptrdiff_t        (* ip_t)(registers);

// These are required by opcodes.h and should not be changed
#define STARTOP(name)       bool name(registers) REGPARM(6);\
                            bool name(registers) {
#define ENDOP               return true; }
#define EXIT(status)        *++sp = status; return false

// This is required by opcode_table.h
#define action(name,param_sz)   {instr(name), param_sz}

// Pull in the opcode definitions
// We pull these into a private namespace so these otherwise common names dont
// pollute the toplevel namespace.
namespace {
#include "opcodes.h"
}

uint32  machine::run(const instr  * program,
                     const byte   * data,
                     uint32       * stack_base, 
                     const size_t   length,
                     Segment & seg, 
                     const int      islot_idx)
{
    assert(program != 0);
    assert(data != 0);
    assert(stack_base !=0);
    assert(length > 8);

    // Declare virtual machine registers
    const instr   * ip = program-1;
    const byte    * dp = data;
    uint32          is = islot_idx, 
                    os = islot_idx;
    // We give enough guard space so that one instruction can over/under flow 
    // the stack and cause no damage this condition will then be caught by
    // check_stack.
    uint32        * sp = stack_base+2;
    uint32 * const  sp_limit = stack_base + length - 2;
  
    // Run the program        
    while((reinterpret_cast<ip_t>(*++ip))(dp, sp, seg, is, os, ip))
    {
#if defined(CHECK_STACK)
        machine::check_stack(sp, stack_base, sp_limit);
#endif
    }

    machine::check_final_stack(sp, stack_base+1, sp_limit);
    return *sp;
}

// Pull in the opcode table
namespace {
#include "opcode_table.h"
}

const opcode_t * machine::get_opcode_table(bool constraint) throw()
{
    return constraint ? opcode_table_constrained : opcode_table;
}


