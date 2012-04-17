/*  GRAPHITE2 LICENSING

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
    If not, write to the Free Software Foundation, 51 Franklin Street, 
    Suite 500, Boston, MA 02110-1335, USA or visit their web page on the 
    internet at http://www.fsf.org/licenses/lgpl.html.

    Alternatively, you may use this library under the terms of the Mozilla
    Public License (http://mozilla.org/MPL) or under the GNU General Public
    License, as published by the Free Sofware Foundation; either version
    2 of the license or (at your option) any later version.
*/

#include "load.h"
#include "loadgr.h"
#include "graphite2/Font.h"
#include "graphite2/Segment.h"
#include "harfbuzz-shaper.h"

#define MAKE_TAG(a, b, c, d) ((gr_uint32)((((gr_uint8)(a)<<24)|((gr_uint8)(b)<<16)|(gr_uint8)(c)<<8)|(gr_uint8)(d)))

gr_uint32 script_tags[] = {
        0                               /* HB_Script_Common */,
        MAKE_TAG('g', 'r', 'e', 'k')    /* HB_Script_Greek */,
        MAKE_TAG('c', 'y', 'r', 'l')    /* HB_Script_Cyrillic */,
        MAKE_TAG('a', 'r', 'm', 'n')    /* HB_Script_Armenian */,
        MAKE_TAG('h', 'e', 'b', 'r')    /* HB_Script_Hebrew */,
        MAKE_TAG('a', 'r', 'a', 'b')    /* HB_Script_Arabic */,
        MAKE_TAG('s', 'y', 'r', 'c')    /* HB_Script_Syriac */,
        MAKE_TAG('t', 'h', 'a', 'a')    /* HB_Script_Thaana */,
        MAKE_TAG('d', 'e', 'v', 'a')    /* HB_Script_Devanagari */,
        MAKE_TAG('b', 'e', 'n', 'g')    /* HB_Script_Bengali */,
        MAKE_TAG('g', 'u', 'r', 'u')    /* HB_Script_Gurmukhi */,
        MAKE_TAG('g', 'u', 'j', 'r')    /* HB_Script_Gujarati */,
        MAKE_TAG('o', 'r', 'y', 'a')    /* HB_Script_Oriya */,
        MAKE_TAG('t', 'a', 'm', 'l')    /* HB_Script_Tamil */,
        MAKE_TAG('t', 'e', 'l', 'u')    /* HB_Script_Telugu */,
        MAKE_TAG('k', 'n', 'd', 'a')    /* HB_Script_Kannada */,
        MAKE_TAG('m', 'l', 'y', 'm')    /* HB_Script_Malayalam */,
        MAKE_TAG('s', 'i', 'n', 'h')    /* HB_Script_Sinhala */,
        MAKE_TAG('t', 'h', 'a', 'i')    /* HB_Script_Thai */,
        MAKE_TAG('l', 'a', 'o', 'o')    /* HB_Script_Lao */,
        MAKE_TAG('t', 'i', 'b', 't')    /* HB_Script_Tibetan */,
        MAKE_TAG('m', 'y', 'm', 'r')    /* HB_Script_Myanmar */,
        MAKE_TAG('g', 'e', 'o', 'r')    /* HB_Script_Georgian */,
        MAKE_TAG('h', 'a', 'n', 'g')    /* HB_Script_Hangul */,
        MAKE_TAG('o', 'g', 'a', 'm')    /* HB_Script_Ogham */,
        MAKE_TAG('r', 'u', 'n', 'r')    /* HB_Script_Runic */,
        MAKE_TAG('k', 'h', 'm', 'r')    /* HB_Script_Khmer */,
        MAKE_TAG('n', 'k', 'o', 'o')    /* HB_Script_Nko */,
        0                               /* HB_Script_Inherited */
};

bool myHB_ShapeItem(HB_Shape_Item *item)
{
    gr_face face = gr_face_from_tf(item->font->userdata->typeface(), 0);
    if (!face) return HB_ShapeItem(item);

    gr_font font = gr_make_font(1.0, face);
    gr_seg seg = gr_make_segment(font, face,
            item->item.script < HB_Script_Inherited ? script_tags[item->item.script] : 0, NULL,
            gr_enc_utf16, item->string + item->item.pos, item->item.length, item->item.bidiLevel & 1);
	if (gr_seg_n_slots(seg) > item->num_glyphs)
	{
		item->num_glyphs = gr_seg_n_slots(seg);
		return 0;
	}

    for (is = gr_seg_first_slot(seg), gp = item->glyphs, cp = item->log_clusters, ap = item->advances;
            is; is = gr_slot_next_in_segment(is), ++gp, ++cp, ++ap)
    {
        *gp = gr_slot_gid(is);
        *ap = gr_slot_advance_X(is);
        if (gr_slot_attached_to(is) && cp > item->log_clusters)
            *cp = cp[-1];
        else
            *cp = gr_cinfo_base(gr_seg_cinfo(seg, gr_slot_before(is)))
    }
    return 1;
}
    
func_map hbmap[] = {
    { "HB_ShapeItem", "myHB_ShapeItem", 0, 0 }
};

