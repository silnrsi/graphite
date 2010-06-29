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
#include <graphiteng/SlotHandle.h>
#include "Machine.h"
#include "Segment.h"

#define STARTOP(name)           name: {
#define ENDOP                   }; goto **++ip;
#define EXIT(status)            push(status); goto end

#define do_(name)               &&name

using namespace vm;

namespace {

const void * direct_run(const bool          get_table_mode,
                        const instr       * program,
                        const byte        * data,
                        Machine::stack_t  * stack_base,
                        Segment     * const seg_ptr,
                        int  &              is)
{
    // We need to define and return to opcode table from within this function 
    // other inorder to take the addresses of the instruction bodies.
    #include "opcode_table.h"
    if (get_table_mode)
        return opcode_table;

    // Declare virtual machine registers
    const instr   * ip = program;
    const byte    * dp = data;
    const int       ib = is;
    // We give enough guard space so that one instruction can over/under flow 
    // the stack and cause no damage this condition will then be caught by
    // check_stack.
    int32        * sp = stack_base + Machine::STACK_GUARD;
    int32 * const  sb = sb;
    Segment &      seg = *seg_ptr;
    
    // start the program
    goto **ip;

    // Pull in the opcode definitions
    #include "opcodes.h"
    
    end:
    return sp;
}

}

const opcode_t * Machine::getOpcodeTable() throw()
{
    int is_dummy;
    return static_cast<const opcode_t *>(direct_run(true, 0, 0, 0, 0, is_dummy));
}


Machine::stack_t  Machine::run(const instr  * program,
                               const byte   * data,
                               Segment      & seg,
                               int          & islot_idx,
                               status_t     & status)
{
    assert(program != 0);
    
    const stack_t *sp = static_cast<const stack_t *>(
                direct_run(false, program, data, _stack, &seg, islot_idx));
    check_final_stack(sp-1, status);
    return *sp;
}

