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


#define registers           const byte * & dp, vm::Machine::stack_t * & sp, vm::Machine::stack_t * const sb,\
                            Segment & seg, int & is, const int ib, \
                            const instr * & ip


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

int32  Machine::run(const instr   * program,
                    const byte    * data,
                    Segment &       seg,
                    int &           is,
                    status_t &      status)
{
    assert(program != 0);

    // Declare virtual machine registers
    const instr   * ip = program-1;
    const byte    * dp = data;
    const int       ib = is;
    // We give enough guard space so that one instruction can over/under flow 
    // the stack and cause no damage this condition will then be caught by
    // check_stack.
    stack_t *       sp      = _stack + Machine::STACK_GUARD,
            * const sb = sp;

    // Run the program        
    while ((reinterpret_cast<ip_t>(*++ip))(dp, sp, sb, seg, is, ib, ip)) {}

    check_final_stack(sp-1, status);

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


