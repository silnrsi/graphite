// This file will be pulled into and integrated into a machine implmentation
// DO NOT build directly
#pragma once

static const opcode_t opcode_table[] = 
{
    action(nop, 0),

    action(push_byte, 1),   action(push_byte_u, 1),
    action(push_short, 2),  action(push_short_u, 2),
    action(push_long, 4),

    action(add, 0), action(sub, 0), action(mul, 0), action(div, 0),
    action(min, 0), action(max, 0), action(neg, 0), action(trunc8, 0), action(trunc16, 0),

    action(cond, 0),    action(and_, 0),    action(or_, 0),     action(not_, 0),
    action(equal, 0),   action(not_eq_, 0),
    action(less, 0),    action(gtr, 0),     action(less_eq, 0), action(gtr_eq, 0),

    action(next, 0), action(next_n, 1), action(copy_next, 0),
    action(put_glyph_8bit_obs, 1),action(put_subs_8bit_obs, 3),
    action(put_copy, 1), action(insert, 0),action(delete_, 0),
    action(assoc, VARARGS), action(cntxt_item, NILOP),

    action(attr_set, 1), action(attr_add, 1), action(attr_sub, 1),
    action(attr_set_slot, 1), action(iattr_set_slot, 2), action(push_slot_attr, 2),
    action(push_glyph_attr_obs, 2), action(push_glyph_metric, 3), action(push_feat, 2),

    action(push_att_to_gattr_obs, 2), action(push_att_to_glyph_metric, 3), 
    action(push_islot_attr, 3),

    action(push_iglyph_attr, NILOP),      // not implemented

    action(pop_ret, 0),action(ret_zero, 0),action(ret_true, 0),

    action(iattr_set, 2), action(iattr_add, 2), action(iattr_sub, 2),
    action(push_proc_state, 1), action(push_version, 0),
    action(put_subs, 5),action(put_subs2, NILOP),action(put_subs3, NILOP),
    action(put_glyph, 2),action(push_glyph_attr, 3),action(push_att_to_glyph_attr, 3)
};

static const opcode_t opcode_table_constrained[] = 
{
    action(nop, 0),

    action(push_byte, 1),   action(push_byte_u, 1),
    action(push_short, 2),  action(push_short_u, 2),
    action(push_long, 4),

    action(add, 0), action(sub, 0), action(mul, 0), action(div, 0),
    action(min, 0), action(max, 0), action(neg, 0), action(trunc8, 0), action(trunc16, 0),

    action(cond, 0),    action(and_, 0),    action(or_, 0),     action(not_, 0),
    action(equal, 0),   action(not_eq_, 0),
    action(less, 0),    action(gtr, 0),     action(less_eq, 0), action(gtr_eq, 0),

    action(next, NILOP), action(next_n, NILOP), action(copy_next, NILOP),
    action(put_glyph_8bit_obs, NILOP),action(put_subs_8bit_obs, NILOP),
    action(put_copy, NILOP), action(insert, NILOP),action(delete_, NILOP),
    action(assoc, NILOP), action(cntxt_item, 2),

    action(attr_set, NILOP), action(attr_add, NILOP), action(attr_sub, NILOP),
    action(attr_set_slot, NILOP), action(iattr_set_slot, NILOP), 
    action(push_slot_attr_constrained, 2),
    action(push_glyph_attr_obs_constrained, 2), 
    action(push_glyph_metric_constrained, 3), 
    action(push_feat_constrained, 2),

    action(push_att_to_gattr_obs_constrained, 2), 
    action(push_att_to_glyph_metric_constrained, 3), 
    action(push_islot_attr_constrained, 3),

    action(push_iglyph_attr, NILOP),      // not implemented

    action(pop_ret, 0),action(ret_zero, 0),action(ret_true, 0),

    action(iattr_set, NILOP), action(iattr_add, NILOP), action(iattr_sub, NILOP),
    action(push_proc_state, 1), action(push_version, 0),
    action(put_subs, NILOP),action(put_subs2, NILOP),action(put_subs3, NILOP),
    action(put_glyph, NILOP),
    action(push_glyph_attr_constrained, 3),
    action(push_att_to_glyph_attr_constrained, 3)
};

