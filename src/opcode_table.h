// This file will be pulled into and integrated into a machine implmentation
// DO NOT build directly
#pragma once

#define do2(n)  do_(n) ,do_(n)
#define NILOP   0U

static const opcode_t opcode_table[] = 
{
    {{do2(nop)},                        0, 0},          // NOP

    {{do2(push_byte)},                  1, 1},          // PUSH_BYTE
    {{do2(push_byte_u)},                1, 1},          // PUSH_BYTE_U
    {{do2(push_short)},                 2, 1},          // PUSH_SHORT
    {{do2(push_short_u)},               2, 1},          // PUSH_SHORT_U
    {{do2(push_long)},                  4, 1},          // PUSH_LONG

    {{do2(add)},                        0, -1},         // ADD
    {{do2(sub)},                        0, -1},         // SUB
    {{do2(mul)},                        0, -1},         // MUL
    {{do2(div)},                        0, -1},         // DIV
    {{do2(min)},                        0, -1},         // MIN
    {{do2(max)},                        0, -1},         // MAX
    {{do2(neg)},                        0, 0},          // NEG
    {{do2(trunc8)},                     0, 0},          // TRUNC8
    {{do2(trunc16)},                    0, 0},          // TRUNC16

    {{do2(cond)},                       0, -2},         // COND
    {{do2(and_)},                       0, -1},         // AND
    {{do2(or_)},                        0, -1},         // OR
    {{do2(not_)},                       0, 0},          // NOT
    {{do2(equal)},                      0, -1},         // EQUAL
    {{do2(not_eq_)},                    0, -1},         // NOT_EQ
    {{do2(less)},                       0, -1},         // LESS
    {{do2(gtr)},                        0, -1},         // GTR
    {{do2(less_eq)},                    0, -1},         // LESS_EQ
    {{do2(gtr_eq)},                     0, -1},         // GTR_EQ

    {{do_(next), NILOP},                0, 0},          // NEXT
    {{do_(next_n), NILOP},              1, 0},          // NEXT_N
    {{do_(copy_next), NILOP},           0, 0},          // COPY_NEXT
    {{do_(put_glyph_8bit_obs), NILOP},  1, 0},          // PUT_GLYPH_8BIT_OBS
    {{do_(put_subs_8bit_obs), NILOP},   3, 0},          // PUT_SUBS_8BIT_OBS
    {{do_(put_copy), NILOP},            1, 0},          // PUT_COPY
    {{do_(insert), NILOP},              0, 0},          // INSERT
    {{do_(delete_), NILOP},             0, 0},          // DELETE
    {{do_(assoc), NILOP},               VARARGS, 0},    // ASSOC
    {{NILOP ,do_(cntxt_item)},          2, 1},          // CNTXT_ITEM

    {{do_(attr_set), NILOP},            1, -1},         // ATTR_SET
    {{do_(attr_add), NILOP},            1, -1},         // ATTR_ADD
    {{do_(attr_sub), NILOP},            1, -1},         // ATTR_SUB
    {{do_(attr_set_slot), NILOP},       1, -1},         // ATTR_SET_SLOT
    {{do_(iattr_set_slot), NILOP},      2, -1},         // IATTR_SET_SLOT
    {{do_(push_slot_attr),
      do_(push_slot_attr_constrained)}, 2, 1},          // PUSH_SLOR_ATTR
    {{do_(push_glyph_attr_obs),
      do_(push_glyph_attr_obs_constrained)}, 2, 1},     // PUSH_GLYPH_ATTR_OBS
    {{do_(push_glyph_metric),
      do_(push_glyph_metric_constrained)}, 3, 1},       // PUSH_GLYPH_METRIC
    {{do_(push_feat),
      do_(push_feat_constrained)},      2, 1},          // PUSH_FEAT

    {{do_(push_att_to_gattr_obs),
      do_(push_att_to_gattr_obs_constrained)}, 2, 1},   // PUSH_ATT_TO_GATTR_OBS
    {{do_(push_att_to_glyph_metric),
      do_(push_att_to_glyph_metric_constrained)}, 3, 1},// PUSH_ATT_TO_GLYPH_METRIC
    {{do_(push_islot_attr),
      do_(push_islot_attr_constrained)}, 3, 1},         // PUSH_ISLOT_ATTR

    {{NILOP,NILOP},                     3, 1},          // PUSH_IGLYPH_ATTR

    {{do2(pop_ret)},                    0, -1},         // POP_RET
    {{do2(ret_zero)},                   0, 0},          // RET_ZERO
    {{do2(ret_true)},                   0, 0},          // RET_TRUE

    {{do_(iattr_set), NILOP},           2, -1},         // IATTR_SET
    {{do_(iattr_add), NILOP},           2, -1},         // IATTR_ADD
    {{do_(iattr_sub), NILOP},           2, -1},         // IATTR_SUB
    {{do2(push_proc_state)},            1, 1},          // PUSH_PROC_STATE
    {{do2(push_version)},               0, 1},          // PUSH_VERSION
    {{do_(put_subs), NILOP},            5, 0},          // PUT_SUBS
    {{NILOP,NILOP},                     0, 0},          // PUT_SUBS2
    {{NILOP,NILOP},                     0, 0},          // PUT_SUBS3
    {{do_(put_glyph), NILOP},           2, 0},          // PUT_GLYPH
    {{do_(push_glyph_attr),
      do_(push_glyph_attr_constrained)}, 3, 1},         // PUSH_GLYPH_ATTR
    {{do_(push_att_to_glyph_attr),
      do_(push_att_to_glyph_attr_constrained)}, 3, 1}   // PUSH_ATT_TO_GLYPH_ATTR
};

