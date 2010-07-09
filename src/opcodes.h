#pragma once
// This file will be pulled into and integrated into a machine implmentation
// DO NOT build directly and under no circumstances every #include headers in 
// here or you will break the direct_machine.
//
// Implementers' notes
// ==================
// You have access to a few primitives and the full C++ code:
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
//    push(n)           Push the value n onto the stack.
//    pop()             Pop the top most value and return it.
//
//    You have access to the following named fast 'registers':
//        sp        = The pointer to the current top of stack, the last value
//                    pushed.
//        seg       = A reference to the Segment this code is running over.
//        is        = The current slot index
//        isb       = The original base slot index at the start of this rule
//        isf       = The first positioned slot
//        isl       = The last positioned slot
//        ip        = The current instruction pointer
//        endPos    = Position of advance of last cluster
     

#define NOT_IMPLEMENTED     assert(false)

#ifdef ENABLE_DEEP_TRACING
#define TRACEPARAM(n)       XmlTraceLog::get().addArrayElement(ElementParams, param, n)
#define TRACEPUSH(n)        XmlTraceLog::get().addSingleElement(ElementPush, n)
#else
#define TRACEPARAM(n)
#define TRACEPUSH(n)
#endif

#define binop(op)           const int32 a = pop(); *sp = int32(*sp) op a; TRACEPUSH(*sp)
#define use_params(n)       dp += n; TRACEPARAM(n)

#define declare_params(n)   const byte * param = dp; \
                            use_params(n);

#define push(n)             *++sp = n; TRACEPUSH(n)
#define pop()               (*sp--)

STARTOP(nop)
    do {} while (0);
ENDOP

STARTOP(push_byte)
    declare_params(1);
    push(int8(*param));
ENDOP

STARTOP(push_byte_u)
    declare_params(1);
    push(uint8(*param));
ENDOP

STARTOP(push_short)
    declare_params(2);
    const int16 r   = int8(param[0]) << 8 
                    | int8(param[1]);
    push(r);
ENDOP

STARTOP(push_short_u)
    declare_params(2);
    const uint16 r  = uint8(param[0]) << 8
                    | uint8(param[1]);
    push(r);
ENDOP

STARTOP(push_long)
    declare_params(4);
    const  int32 r  = uint8(param[0]) << 24
                    | uint8(param[1]) << 16
                    | uint8(param[2]) << 8
                    | uint8(param[3]);
    push(r);
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

STARTOP(div_)
    binop(/);
ENDOP

STARTOP(min)
    const int32 a = pop(), b = *sp;
    if (a < b) *sp = a;
ENDOP

STARTOP(max)
    const int32 a = pop(), b = *sp;
    if (a > b) *sp = a;
ENDOP

STARTOP(neg)
    *sp = uint32(-int32(*sp));
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
    ++is;
ENDOP

STARTOP(next_n)
    declare_params(1);
    const size_t    count = uint8(*param);
    // TODO: In the original graphite this always asserts to false: check.
    NOT_IMPLEMENTED;
ENDOP

STARTOP(copy_next)
    is++;
ENDOP

STARTOP(put_glyph_8bit_obs)
    declare_params(1);
    const unsigned int output_class = uint8(*param);
    seg[is].setGlyph(&seg, seg.getClassGlyph(output_class, 0));
ENDOP

STARTOP(put_subs_8bit_obs)
    declare_params(3);
    const int           slot_ref     = int8(param[0]);
    const unsigned int  input_class  = uint8(param[1]),
                        output_class = uint8(param[2]);
    uint16 index = seg.findClassIndex(input_class, seg[is + slot_ref].gid());
    seg[is].setGlyph(&seg, seg.getClassGlyph(output_class, index));
ENDOP

STARTOP(put_copy)
    declare_params(1);
    const unsigned int  slot_ref = uint8(*param);
    if (slot_ref)
    {
	memcpy(&(seg[is]), &(seg[is + slot_ref]), sizeof(Slot));
        seg.copyUserAttrs(is, is + slot_ref);
    }
ENDOP

STARTOP(insert)
    seg.insertSlot(is);
    seg[is].originate(seg[is + 1].original());
ENDOP

STARTOP(delete_)
    seg[is].markDeleted(true);
ENDOP

STARTOP(assoc)
    declare_params(1);
    unsigned int  count = uint8(*param);
    const int8 *        assocs = reinterpret_cast<const int8 *>(param+1);
    use_params(count);
    int max = -1;
    int min = -1;
    
    while (count-- > 0)
    {
        int ts = is + *assocs++;
        if (min == -1 || seg[ts].before() < min) min = seg[ts].before();
        if (seg[ts].after() > max) max = seg[ts].after();
    }
    seg[is].before(min);
    seg[is].after(max);
ENDOP

STARTOP(cntxt_item)
    // It turns out this is a cunningly disguised condition forward jump.
    // TODO: Put checks for this one to avoid jumping off the end of the program.
    declare_params(3);    
    const int       is_arg = int8(param[0]);
    const size_t    iskip  = uint8(param[1]),
                    dskip  = uint8(param[2]);

    if (isb + is_arg != is) {
        ip += iskip;
        dp += dskip;
        push(true);
    }
ENDOP

STARTOP(attr_set)
    declare_params(1);
    const attrCode  	slat = attrCode(uint8(*param));
    const          int  val  = int(pop());
    seg[is].setAttr(&seg, slat, 0, val, is);
ENDOP

STARTOP(attr_add)
    declare_params(1);
    const attrCode  	slat = attrCode(uint8(*param));
    const          int  val  = int(pop());
    int res = seg[is].getAttr(&seg, slat, 0, is, &isf, &isl, &endPos);
    seg[is].setAttr(&seg, slat, 0, val + res, is);
ENDOP

STARTOP(attr_sub)
    declare_params(1);
    const attrCode  	slat = attrCode(uint8(*param));
    const          int  val  = int(pop());
    int res = seg[is].getAttr(&seg, slat, 0, is, &isf, &isl, &endPos);
    seg[is].setAttr(&seg, slat, 0, res - val, is);
ENDOP

STARTOP(attr_set_slot)
    declare_params(1);
    const attrCode  	slat = attrCode(uint8(*param));
    const          int  val  = int(pop());
    seg[is].setAttr(&seg, slat, 0, val + is, is);
ENDOP

STARTOP(iattr_set_slot)
    declare_params(2);
    const attrCode  	slat = attrCode(uint8(param[0]));
    const size_t        idx  = uint8(param[1]);
    const          int  val  = int(pop());
    seg[is].setAttr(&seg, slat, idx, val + is, is);
ENDOP

STARTOP(push_slot_attr)
    declare_params(2);
    const attrCode  	slat     = attrCode(uint8(param[0]));
    const int           slot_ref = int8(param[1]);
    push(seg[is + slot_ref].getAttr(&seg, slat, 0, is + slot_ref, &isf, &isl, &endPos));
ENDOP

STARTOP(push_slot_attr_constrained)
    declare_params(2);
    const attrCode  	slat     = attrCode(uint8(param[0]));
    const int           slot_ref = int8(param[1]);
    push(seg[is + slot_ref].getAttr(&seg, slat, 0, is + slot_ref, &isf, &isl, &endPos));
ENDOP

STARTOP(push_glyph_attr_obs)
    declare_params(2);
    const unsigned int  glyph_attr  = uint8(param[0]);
    const int           slot_ref    = int8(param[1]);
    push(seg.glyphAttr(seg[is + slot_ref].gid(), glyph_attr));
ENDOP

STARTOP(push_glyph_attr_obs_constrained)
    declare_params(2);
    const unsigned int  glyph_attr  = uint8(param[0]);
    const int           slot_ref    = int8(param[1]);
    push(seg.glyphAttr(seg[is + slot_ref].gid(), glyph_attr));
ENDOP

STARTOP(push_glyph_metric)
    declare_params(3);
    const unsigned int  glyph_attr  = uint8(param[0]);
    const int           slot_ref    = int8(param[1]);
    const signed int    attr_level  = uint8(param[2]);
    push(seg.getGlyphMetric(is + slot_ref, glyph_attr, attr_level));
ENDOP

STARTOP(push_glyph_metric_constrained)
    declare_params(3);
    const unsigned int  glyph_attr  = uint8(param[0]);
    const int           slot_ref    = int8(param[1]);
    const signed int    attr_level  = uint8(param[2]);
    push(seg.getGlyphMetric(is + slot_ref, glyph_attr, attr_level));
ENDOP

STARTOP(push_feat)
    declare_params(2);
    const unsigned int  feat        = uint8(param[0]);
    const int           slot_ref    = int8(param[1]);
    uint8 fid = seg.charinfo(seg[is + slot_ref].original())->fid();
    push(seg.getFeature(fid, feat));
ENDOP

STARTOP(push_feat_constrained)
    declare_params(2);
    const unsigned int  feat        = uint8(param[0]);
    const int           slot_ref    = int8(param[1]);
    uint8 fid = seg.charinfo(seg[is + slot_ref].original())->fid();
    push(seg.getFeature(fid, feat));
ENDOP

STARTOP(push_att_to_gattr_obs)
    declare_params(2);
    const unsigned int  glyph_attr  = uint8(param[0]);
    const int           slot_ref    = int8(param[1]);
    int slot = seg[is + slot_ref].attachTo();
    if (slot < 0) slot = is + slot_ref;
    push(seg.glyphAttr(seg[slot].gid(), glyph_attr));
ENDOP

STARTOP(push_att_to_gattr_obs_constrained)
    declare_params(2);
    const unsigned int  glyph_attr  = uint8(param[0]);
    const int           slot_ref    = int8(param[1]);
    int slot = seg[is + slot_ref].attachTo();
    if (slot < 0) slot = is + slot_ref;
    push(seg.glyphAttr(seg[slot].gid(), glyph_attr));
ENDOP

STARTOP(push_att_to_glyph_metric)
    declare_params(3);
    const unsigned int  glyph_attr  = uint8(param[0]);
    const int           slot_ref    = int8(param[1]);
    const signed int    attr_level  = uint8(param[2]);
    int slot = seg[is + slot_ref].attachTo();
    if (slot < 0) slot = is + slot_ref;
    push(seg.getGlyphMetric(slot, glyph_attr, attr_level));
ENDOP

STARTOP(push_att_to_glyph_metric_constrained)
    declare_params(3);
    const unsigned int  glyph_attr  = uint8(param[0]);
    const int           slot_ref    = int8(param[1]);
    const signed int    attr_level  = uint8(param[2]);
    int slot = seg[is + slot_ref].attachTo();
    if (slot < 0) slot = is + slot_ref;
    push(seg.getGlyphMetric(slot, glyph_attr, attr_level));
ENDOP

STARTOP(push_islot_attr)
    declare_params(3);
    const attrCode	slat     = attrCode(uint8(param[0]));
    const int           slot_ref = int8(param[1]),
                        idx      = uint8(param[2]);
    push(seg[is + slot_ref].getAttr(&seg, slat, idx, is + slot_ref, &isf, &isl, &endPos));
ENDOP

STARTOP(push_islot_attr_constrained)
    declare_params(3);
    const attrCode  	slat     = attrCode(uint8(param[0]));
    const int           slot_ref = int8(param[1]),
                        idx      = uint8(param[2]);
    push(seg[is + slot_ref].getAttr(&seg, slat, idx, is + slot_ref, &isf, &isl, &endPos));
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
    const attrCode  	slat = attrCode(uint8(param[0]));
    const size_t        idx  = uint8(param[1]);
    const          int  val  = int(pop());
    seg[is].setAttr(&seg, slat, idx, val, is);
ENDOP

STARTOP(iattr_add)
    declare_params(2);
    const attrCode  	slat = attrCode(uint8(param[0]));
    const size_t        idx  = uint8(param[1]);
    const          int  val  = int(pop());
    int res = seg[is].getAttr(&seg, slat, idx, is, &isf, &isl, &endPos);
    seg[is].setAttr(&seg, slat, idx, val + res, is);
ENDOP

STARTOP(iattr_sub)
    declare_params(2);
    const attrCode  	slat = attrCode(uint8(param[0]));
    const size_t        idx  = uint8(param[1]);
    const          int  val  = int(pop());
    int res = seg[is].getAttr(&seg, slat, idx, is, &isf, &isl, &endPos);
    seg[is].setAttr(&seg, slat, idx, val - res, is);
ENDOP

STARTOP(push_proc_state)
    declare_params(1);
    const unsigned int  pstate = uint8(*param);
    // TODO; Implement body
    push(1);
ENDOP

STARTOP(push_version)
    push(0x00030000);
ENDOP

STARTOP(put_subs)
    declare_params(5);
    const int        slot_ref     = int8(param[0]);
    const unsigned int  input_class  = uint8(param[1]) << 8
                                     | uint8(param[2]);
    const unsigned int  output_class = uint8(param[3]) << 8
                                     | uint8(param[4]);
    int index = seg.findClassIndex(input_class, seg[is + slot_ref].gid());
    seg[is].setGlyph(&seg, seg.getClassGlyph(output_class, index));
ENDOP

STARTOP(put_subs2) // not implemented
    NOT_IMPLEMENTED;
ENDOP

STARTOP(put_subs3) // not implemented
    NOT_IMPLEMENTED;
ENDOP

STARTOP(put_glyph)
    declare_params(2);
    const unsigned int output_class  = uint8(param[0]) << 8
                                     | uint8(param[1]);
    seg[is].setGlyph(&seg, seg.getClassGlyph(output_class, 0));
ENDOP

STARTOP(push_glyph_attr)
    declare_params(3);
    const unsigned int  glyph_attr  = uint8(param[0]) << 8
                                    | uint8(param[1]);
    const int           slot_ref    = int8(param[2]);
    push(seg.glyphAttr(seg[is + slot_ref].gid(), glyph_attr));
ENDOP

STARTOP(push_glyph_attr_constrained)
    declare_params(3);
    const unsigned int  glyph_attr  = uint8(param[0]) << 8
                                    | uint8(param[1]);
    const int           slot_ref    = int8(param[2]);
    push(seg.glyphAttr(seg[is + slot_ref].gid(), glyph_attr));
ENDOP

STARTOP(push_att_to_glyph_attr)
    declare_params(3);
    const unsigned int  glyph_attr  = uint8(param[0]) << 8
                                    | uint8(param[1]);
    const int           slot_ref    = int8(param[2]);
    int slot = seg[is + slot_ref].attachTo();
    if (slot < 0) slot = is + slot_ref;
    push(seg.glyphAttr(seg[slot].gid(), glyph_attr));
ENDOP

STARTOP(push_att_to_glyph_attr_constrained)
    declare_params(3);
    const unsigned int  glyph_attr  = uint8(param[0]) << 8
                                    | uint8(param[1]);
    const int           slot_ref    = int8(param[2]);
    int slot = seg[is + slot_ref].attachTo();
    if (slot < 0) slot = is + slot_ref;
    push(seg.glyphAttr(seg[slot].gid(), glyph_attr));
ENDOP

