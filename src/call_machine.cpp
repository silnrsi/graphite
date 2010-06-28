// This call threaded interpreter implmentation for machine.h
// Author: Tim Eves

// Build either this interpreter or the direct_machine implementation.
// The call threaded interpreter is portable across compilers and 
// architectures as well as being useful to debug (you can set breakpoints on 
// opcodes) but is slower that the direct threaded interpreter by a factor of 2

#include <cassert>
#include <graphiteng/SlotHandle.h>
#include "Machine.h"
#include "Segment.h"
#include "XmlTraceLog.h"


#define registers           const byte * & dp, int32 * & sp, Segment & seg, \
                            int & is, const int ib, const instr * & ip


// These are required by opcodes.h and should not be changed
#define STARTOP(name)	    bool name(registers) REGPARM(6);\
			                bool name(registers) { STARTTRACE(name,is)
#define ENDOP		        ENDTRACE return true; }
#define EXIT(status)        push(status); ENDTRACE return false

// This is required by opcode_table.h
#define do_(name)           instr(name)


using namespace vm;

typedef ptrdiff_t        (* ip_t)(registers);

// Pull in the opcode definitions
// We pull these into a private namespace so these otherwise common names dont
// pollute the toplevel namespace.
namespace {
#include "opcodes.h"
}

int32  Machine::run(const instr  * program,
                     const byte   * data,
                     int32       * stack_base, 
                     const size_t   length,
                     Segment &      seg, 
                     int &          is,
                     status_t &     status)
{
    assert(program != 0);
    assert(stack_base !=0);
    assert(length > 8);

    // Declare virtual machine registers
    const instr   * ip = program-1;
    const byte    * dp = data;
    const int       ib = is;
    // We give enough guard space so that one instruction can over/under flow 
    // the stack and cause no damage this condition will then be caught by
    // check_stack.
    int32        * sp        = stack_base + length - 2;
    int32 * const  stack_top = stack_base + 2;
    stack_base = sp;

    // Run the program        
    while ((reinterpret_cast<ip_t>(*++ip))(dp, sp, seg, is, ib, ip)
#if defined(CHECK_STACK)
           && check_stack(sp, stack_base, stack_top)
#endif
           ) {}

    check_final_stack(sp, stack_base-1, stack_top, status);
    return *sp;
}

// Pull in the opcode table
namespace {
#include "opcode_table.h"
}

const opcode_t * Machine::getOpcodeTable() throw()
{
    return opcode_table;
}


