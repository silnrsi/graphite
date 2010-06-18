// This general interpreter interface.
// Author: Tim Eves

// Build one of direct_machine.cpp or call_machine.cpp to implement this 
// interface.

#pragma once
#include <stdexcept>
#include <graphiteng/Types.h>

#define CHECK_STACK   TRUE

// Forward declarations
class Segment;

typedef void *  instr;

struct opcode_t 
{ 
    const instr     impl; 
    const size_t    param_sz;
};

enum { VARARGS = size_t(-2), NILOP };

namespace machine
{
    extern const opcode_t *    get_opcode_table(bool constraint) throw();
    extern uint32              run(const instr * program, 
                                   uint32 * stack_base, const size_t length,
                                   Segment & seg, const int islot_idx);

    inline void check_stack(const uint32 * const sp, 
                            const uint32 * const base,
                            const uint32 * const limit) {
            if (sp <= base+2)     throw std::runtime_error("check_stack: stack underflow");
            if (sp >= limit-2)    throw std::runtime_error("check_stack: stack overflow");
    }

    inline void check_final_stack(const uint32 * const sp, 
                            const uint32 * const base,
                            const uint32 * const limit) {
        if (sp > base + 2)
            throw std::runtime_error("check_final_stack: stack not emptied");
        if (sp < base + 2)
            throw std::runtime_error("check_final_stack: stack underflowed");
        if (sp >= limit - 2)
            throw std::runtime_error("check_final_stack: stack overflowed");
    }

    enum opcode {
	    NOP = 0,

	    PUSH_BYTE,		PUSH_BYTEU,		PUSH_SHORT,	PUSH_SHORTU,	PUSH_LONG,

	    ADD,				SUB,				MUL,			DIV,
	    MIN_,				MAX_,
	    NEG,
	    TRUNC8,			TRUNC16,

	    COND,
	
	    AND,				OR,				NOT,
	    EQUAL,			NOT_EQ,
	    LESS,			GTR,				LESS_EQ,		GTR_EQ,

	    NEXT,			NEXTN,			COPY_NEXT,
	    PUT_GLYPH8BIT_OBS,	PUT_SUBS8BIT_OBS,	PUT_COPY,
	    INSERT,			DELETE,
	    ASSOC,
	    CNTXT_ITEM,

	    ATTR_SET,			ATTR_ADD,			ATTR_SUB,
	    ATTR_SET_SLOT,
	    I_ATTR_SET_SLOT,
	    PUSH_SLOT_ATTR,	PUSH_GLYPH_ATTR_OBS,_PUSH_GLYPH_METRIC,		PUSH_FEAT,
	    PUSH_ATT_TOG_ATTR_OBS,	PUSH_ATT_TOGLYPH_METRIC,
	    PUSHI_SLOT_ATTR,

	    PUSHI_GLYPH_ATTR,	// not implemented

	    POP_RET,			RET_ZERO,			RET_TRUE,
	    I_ATTR_SET,		I_ATTR_ADD,		I_ATTR_SUB,
	    PUSH_PROC_STATE,	PUSH_VERSION,
	    PUT_SUBS,			PUT_SUBS2,		PUT_SUBS3,
	    PUT_GLYPH,		PUSH_GLYPH_ATTR,	PUSH_ATT_TOGLYPH_ATTR,
	    MAX_OPCODE
    };
}

#define use_params(n)       ip += (n + sizeof(instr)-1)/sizeof(instr)
#define declare_params(n)   const byte * param = \
                                    reinterpret_cast<const byte *>(ip+1); \
                            use_params(n);
  



