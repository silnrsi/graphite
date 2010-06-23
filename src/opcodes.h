// This file will be pulled into and integrated into a machine implmentation
// DO NOT build directly
//
// Implementers notes
// ==================
// You have at you displsoal access to a few primatives and the full C++ code:
//    declare_params(n) Tells the interpreter how many bytes of parameter
//                      space to claim for this instruction uses and 
//                      initialises the param pointer.  You *must* before the 
//                      first use of param.
//    use_params(n)     Claim n extra bytes of param space beyond what was 
//                      claimed using delcare_param.
//    param             A const byte pointer for the parameter space claimed by
//                      this instruction.
//    binop(op)         Implement a binary operation on the stack using the 
//                      specified C++ operator.
//    NOT_IMPLEMENTED   Any instruction body containing this will exit the 
//                      program with an assertion error.  Instructions that are
//                      not implemented should also be marked NILOP in the
//                      opcodes tables this will cause the code class to spot
//                      them in a live code stream and throw a runtime_error 
//                      instead.
//
//    You have access to the following named 'registers':
//        ip    = The current virtual instruction pointer. This always points to the 
//                instruction currently executing.
//        sp    = The pointer to the current top of stack.
//        seg   = A reference to the Segment this code is running over.
//        is    = The input slot index
//        os    = The output slot index
//        param = pointer to the instructions parameter block.
//                (Only if you use declare_params first).
     
     
#define binop(op)           const uint32 a = pop(); *sp = *sp op a

#define NOT_IMPLEMENTED     assert(false)
#define use_params(n)       dp += n
#define declare_params(n)   const byte * param = dp; \
                            use_params(n);
#define push(n)             *--sp = n
#define pop()               *sp++
#define drop(n)             sp += n

STARTOP(nop)
    do {} while (0);
ENDOP

STARTOP(push_byte)
    declare_params(1);
    push(signed(*param));
ENDOP

STARTOP(push_byte_u)
    declare_params(1);
    push(unsigned(*param));
ENDOP

STARTOP(push_short)
    declare_params(2);
    const uint16 r  = param[0] << 8 
                    | param[1];
    push(signed(r));
ENDOP

STARTOP(push_short_u)
    declare_params(2);
    const uint16 r  = param[0] << 8
                    | param[1];
    push(unsigned(r));
ENDOP

STARTOP(push_long)
    declare_params(4);
    const uint32 r  = param[0] << 24 
                    | param[1] << 16
                    | param[2] << 8
                    | param[4];
    push(unsigned(r));
ENDOP

STARTOP(add)
    binop(+);
ENDOP

STARTOP(sub)
    binop(-);
ENDOP

STARTOP(mul)
    binop(*);
ENDOP

STARTOP(div)
    binop(/);
ENDOP

STARTOP(min)
    const uint32 a = pop(), b = *sp;
    if (a < b) *sp = a;
ENDOP

STARTOP(max)
    const uint32 a = pop(), b = *sp;
    if (a > b) *sp = a;
ENDOP

STARTOP(neg)
    *sp = -*sp;
ENDOP

STARTOP(trunc8)
    *sp = uint8(*sp);
ENDOP

STARTOP(trunc16)
    *sp = uint16(*sp);
ENDOP

STARTOP(cond)
    const uint32 c = pop(), t = pop();
    if (c) *sp = t;
ENDOP

STARTOP(and_)
    binop(&&);
ENDOP

STARTOP(or_)
    binop(||);
ENDOP

STARTOP(not_)
    *sp = !*sp;
ENDOP

STARTOP(equal)
    binop(==);
ENDOP

STARTOP(not_eq_)
    binop(!=);
ENDOP

STARTOP(less)
    binop(<);
ENDOP

STARTOP(gtr)
    binop(>);
ENDOP

STARTOP(less_eq)
    binop(<=);
ENDOP

STARTOP(gtr_eq)
    binop(>=);
ENDOP

STARTOP(next)
    // TODO: In the original graphite code this appears to do nothing: check.
    is++;
ENDOP

STARTOP(next_n)
    declare_params(1);
    const size_t    count = size_t(*param);
    // TODO: In the original graphite this always asserts to false: check.
    NOT_IMPLEMENTED;
ENDOP

STARTOP(copy_next)
    // TODO: Implement copy_next
    is++;
ENDOP

STARTOP(put_glyph_8bit_obs)
    declare_params(1);
    const unsigned int output_class = unsigned(*param);
    // TODO: Implement body
ENDOP

STARTOP(put_subs_8bit_obs)
    declare_params(3);
    const size_t        slot_ref     = size_t(param[0]);
    const unsigned int  input_class  = unsigned(param[1]),
                        output_class = unsigned(param[2]);
    // TODO; Implement body
ENDOP

STARTOP(put_copy)
    declare_params(1);
    const unsigned int  slot_ref = unsigned(*param);
    // TODO; Implement body
ENDOP

STARTOP(insert)
    seg.insertSlot(is);
    // TODO; Implement body;
ENDOP

STARTOP(delete_)
    seg[is].markDeleted(true);
    // TODO; Implement body;
ENDOP

STARTOP(assoc)
    declare_params(1);
    const unsigned int  count = unsigned(*param);
    const int8 * assocs = reinterpret_cast<const int8 *>(param+1);
    use_params(count);
    
    // TODO; Implement body;   
ENDOP

STARTOP(cntxt_item)
    // It turns out this is a cunningly disguised condition forward jump.
    // TODO: Put checks for this one to avoid jumping off the end of the program.
    declare_params(2);    
    const size_t    is_arg = size_t(param[0]);
    const size_t    count  = size_t(param[1]);

    if (is_arg != is) {
        ip += count;
        push(true);
    }
ENDOP

STARTOP(attr_set)
    declare_params(1);
    const unsigned int  slat = unsigned(*param);
    const          int  val  = int(pop());

    // TODO; Implement body
ENDOP

STARTOP(attr_add)
    declare_params(1);
    const unsigned int  slat = unsigned(*param);
    const          int  val  = int(pop());

    // TODO; Implement body
ENDOP

STARTOP(attr_sub)
    declare_params(1);
    const unsigned int  slat = unsigned(*param);
    const          int  val  = int(pop());

    // TODO; Implement body
ENDOP

STARTOP(attr_set_slot)
    declare_params(1);
    const unsigned int  slat = unsigned(*param);
    const          int  val  = int(pop());

    // TODO; Implement body
ENDOP

STARTOP(iattr_set_slot)
    declare_params(2);
    const unsigned int  slat = unsigned(param[0]);
    const size_t        idx  = size_t(param[1]);
    const          int  val  = int(pop());

    // TODO; Implement body
ENDOP

STARTOP(push_slot_attr)
    declare_params(2);
    const unsigned int  slat     = unsigned(param[0]);
    const size_t        slot_ref = size_t(param[1]);
    // TODO; Implement body
    push(0);
ENDOP

STARTOP(push_slot_attr_constrained)
    declare_params(2);
    const unsigned int  slat     = unsigned(param[0]);
    const size_t        slot_ref = size_t(param[1]) + is + 1;
    // TODO; Implement body
    push(0);
ENDOP

STARTOP(push_glyph_attr_obs)
    declare_params(2);
    const unsigned int  glyph_attr  = unsigned(param[0]);
    const size_t        slot_ref    = size_t(param[1]);
    // TODO; Implement body
    push(0);
ENDOP

STARTOP(push_glyph_attr_obs_constrained)
    declare_params(2);
    const unsigned int  glyph_attr  = unsigned(param[0]);
    const size_t        slot_ref    = size_t(param[1]) + is + 1;
    // TODO; Implement body
    push(0);
ENDOP

STARTOP(push_glyph_metric)
    declare_params(3);
    const unsigned int  glyph_attr  = unsigned(param[0]);
    const size_t        slot_ref    = size_t(param[1]);
    const signed int    attr_level  = signed(param[2]);
    // TODO; Implement body
    push(0);
ENDOP

STARTOP(push_glyph_metric_constrained)
    declare_params(3);
    const unsigned int  glyph_attr  = unsigned(param[0]);
    const size_t        slot_ref    = size_t(param[1]) + is + 1;
    const signed int    attr_level  = signed(param[2]);
    // TODO; Implement body
    push(0);
ENDOP

STARTOP(push_feat)
    declare_params(2);
    const unsigned int  feat        = unsigned(param[0]);
    const size_t        slot_ref    = size_t(param[1]);
    // TODO; Implement body
    push(0);
ENDOP

STARTOP(push_feat_constrained)
    declare_params(2);
    const unsigned int  feat        = unsigned(param[0]);
    const size_t        slot_ref    = size_t(param[1]) + is + 1;
    // TODO; Implement body
    push(0);
ENDOP

STARTOP(push_att_to_gattr_obs)
    declare_params(2);
    const unsigned int  glyph_attr  = unsigned(param[0]);
    const size_t        slot_ref    = size_t(param[1]);
    // TODO; Implement body
    push(0);
ENDOP

STARTOP(push_att_to_gattr_obs_constrained)
    declare_params(2);
    const unsigned int  glyph_attr  = unsigned(param[0]);
    const size_t        slot_ref    = size_t(param[1]) + is + 1;
    // TODO; Implement body
    push(0);
ENDOP

STARTOP(push_att_to_glyph_metric)
    declare_params(3);
    const unsigned int  glyph_attr  = unsigned(param[0]);
    const size_t        slot_ref    = size_t(param[1]);
    const signed int    attr_level  = signed(param[2]);
    // TODO; Implement body
    push(0);
ENDOP

STARTOP(push_att_to_glyph_metric_constrained)
    declare_params(3);
    const unsigned int  glyph_attr  = unsigned(param[0]);
    const size_t        slot_ref    = size_t(param[1]) + is + 1;
    const signed int    attr_level  = signed(param[2]);
    // TODO; Implement body
    push(0);
ENDOP

STARTOP(push_islot_attr)
    declare_params(3);
    const unsigned int  slat     = unsigned(param[0]);
    const size_t        slot_ref = size_t(param[1]),
                        idx      = size_t(param[2]);
    // TODO; Implement body
    push(0);
ENDOP

STARTOP(push_islot_attr_constrained)
    declare_params(3);
    const unsigned int  slat     = unsigned(param[0]);
    const size_t        slot_ref = size_t(param[1]) + is + 1,
                        idx      = size_t(param[2]);
    // TODO; Implement body
    push(0);
ENDOP

STARTOP(push_iglyph_attr) // not implemented
    NOT_IMPLEMENTED;
ENDOP
      
STARTOP(pop_ret)
    const uint32 ret = pop();
    EXIT(ret);
ENDOP

STARTOP(ret_zero)
    EXIT(0);
ENDOP

STARTOP(ret_true)
    EXIT(1);
ENDOP

STARTOP(iattr_set)
    declare_params(2);
    const unsigned int  slat = unsigned(param[0]);
    const size_t        idx  = size_t(param[1]);
    const          int  val  = int(pop());

    // TODO; Implement body
ENDOP

STARTOP(iattr_add)
    declare_params(2);
    const unsigned int  slat = unsigned(param[0]);
    const size_t        idx  = size_t(param[1]);
    const          int  val  = int(pop());

    // TODO; Implement body
ENDOP

STARTOP(iattr_sub)
    declare_params(2);
    const unsigned int  slat = unsigned(param[0]);
    const size_t        idx  = size_t(param[1]);
    const          int  val  = int(pop());

    // TODO; Implement body
ENDOP

STARTOP(push_proc_state)
    declare_params(1);
    const unsigned int  pstate = *param;
    // TODO; Implement body
    push(1);
ENDOP

STARTOP(push_version)
    push(0x00030000);
ENDOP

STARTOP(put_subs)
    declare_params(5);
    const size_t        slot_ref     = size_t(param[0]);
    const unsigned int  input_class  = unsigned(param[1] << 8) 
                                     | unsigned(param[2]);
    const unsigned int  output_class = unsigned(param[3] << 8) 
                                     | unsigned(param[4]);
    // TODO; Implement body
ENDOP

STARTOP(put_subs2) // not implemented
    NOT_IMPLEMENTED;
ENDOP

STARTOP(put_subs3) // not implemented
    NOT_IMPLEMENTED;
ENDOP

STARTOP(put_glyph)
    declare_params(2);
    const unsigned int output_class  = unsigned(param[0] << 8) 
                                     | unsigned(param[1]);
    // TODO: Implement body
ENDOP

STARTOP(push_glyph_attr)
    declare_params(3);
    const unsigned int  glyph_attr  = unsigned(param[0])
                                    | unsigned(param[1]);
    const size_t        slot_ref    = size_t(param[2]);
    // TODO; Implement body
    push(0);
ENDOP

STARTOP(push_glyph_attr_constrained)
    declare_params(3);
    const unsigned int  glyph_attr  = unsigned(param[0])
                                    | unsigned(param[1]);
    const size_t        slot_ref    = size_t(param[2]) + is + 1;
    // TODO; Implement body
    push(0);
ENDOP

STARTOP(push_att_to_glyph_attr)
    declare_params(3);
    const unsigned int  glyph_attr  = unsigned(param[0])
                                    | unsigned(param[1]);
    const size_t        slot_ref    = size_t(param[2]);
    // TODO; Implement body
    push(0);
ENDOP

STARTOP(push_att_to_glyph_attr_constrained)
    declare_params(3);
    const unsigned int  glyph_attr  = unsigned(param[0])
                                    | unsigned(param[1]);
    const size_t        slot_ref    = size_t(param[2]) + is + 1;
    // TODO; Implement body
    push(0);
ENDOP

