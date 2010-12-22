/*  GRAPHITENG LICENSING

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
    If not, write to the Free Software Foundation, Inc., 59 Temple Place,
    Suite 330, Boston, MA 02111-1307, USA or visit their web page on the
    internet at http://www.fsf.org/licenses/lgpl.html.
*/
#pragma once

#include "graphite2/Types.h"
#include "graphite2/Font.h"

#ifdef __cplusplus
extern "C"
{
#endif

enum gr_break_weight {
    gr_breakNone = 0,
    /* after break weights */
    gr_breakWhitespace = 10,
    gr_breakWord = 15,
    gr_breakIntra = 20,
    gr_breakLetter = 30,
    gr_breakClip = 40,
    /* before break weights */
    gr_breakBeforeWhitespace = -10,
    gr_breakBeforeWord = -15,
    gr_breakBeforeIntra = -20,
    gr_breakBeforeLetter = -30,
    gr_breakBeforeClip = -40
};

/** Used for looking up slot attributes. Most are already available in other functions **/
enum gr_attrCode {
    /// adjusted glyph advance in x direction
    gr_slatAdvX = 0,        
    /// adjusted glyph advance in y direction (usually 0)
    gr_slatAdvY,            
    /// returns 0. Deprecated.
    gr_slatAttTo,           
    /// This slot attaches to its parent at the given design units in the x direction
    gr_slatAttX,            
    /// This slot attaches to its parent at the given design units in the y direction
    gr_slatAttY,            
    /// This slot attaches to its parent at the given glyph point (not implemented)
    gr_slatAttGpt,          
    /// x-direction adjustment from the given glyph point (not implemented)
    gr_slatAttXOff,         
    /// y-direction adjustment from the given glyph point (not implemented)
    gr_slatAttYOff,         
    /// Where on this glyph should align with the attachment point on the parent glyph in the x-direction.
    gr_slatAttWithX,        
    /// Where on this glyph should align with the attachment point on the parent glyph in the y-direction
    gr_slatAttWithY,        
    /// Which glyph point on this glyph should align with the attachment point on the parent glyph (not implemented).
    gr_slatWithGpt,         
    /// Adjustment to gr_slatWithGpt in x-direction (not implemented)
    gr_slatAttWithXOff,     
    /// Adjustment to gr_slatWithGpt in y-direction (not implemented)
    gr_slatAttWithYOff,     
    /// Attach at given nesting level (not implemented)
    gr_slatAttLevel,        
    /// Line break breakweight for this glyph
    gr_slatBreak,           
    /// Ligature component reference (not implemented)
    gr_slatCompRef,         
    /// bidi directionality of this glyph (not implemented)
    gr_slatDir,             
    /// Whether insertion is allowed before this glyph
    gr_slatInsert,          
    /// Final positioned position of this glyph relative to its parent in x-direction in pixels
    gr_slatPosX,            
    /// Final positioned position of this glyph relative to its parent in y-direction in pixels
    gr_slatPosY,            
    /// Amount to shift glyph by in x-direction design units
    gr_slatShiftX,          
    /// Amount to shift glyph by in y-direction design units
    gr_slatShiftY,          
    /// attribute user1
    gr_slatUserDefnV1,      
    /// not implemented
    gr_slatMeasureSol,      
    /// not implemented
    gr_slatMeasureEol,      
    /// Amount this slot can stretch (not implemented)
    gr_slatJStretch,        
    /// Amount this slot can shrink (not implemented)
    gr_slatJShrink,         
    /// Granularity by which this slot can stretch or shrink (not implemented)
    gr_slatJStep,           
    /// Justification weight for this glyph (not implemented)
    gr_slatJWeight,         
    /// Amount this slot mush shrink or stretch (not implemented)
    gr_slatJWidth,          
    /// User defined attribute, see subattr for user attr number
    gr_slatUserDefn = gr_slatJStretch + 30,
                            
    /// not implemented
    gr_slatMax,             
    /// not implemented
    gr_slatNoEffect = gr_slatMax + 1    
};


typedef struct gr_char_info     gr_char_info;
typedef struct gr_segment       gr_segment;
typedef struct gr_slot          gr_slot;

/** Returns Unicode character for a charinfo.
  * 
  * @param p Pointer to charinfo to return information on.
  */
GRNG_EXPORT unsigned int gr_cinfo_unicode_char(const gr_char_info* p/*not NULL*/);

/** Returns breakweight for a charinfo.
  * 
  * @return Breakweight is a number between -50 and 50 indicating the cost of a
  * break before or after this character.
  * @param p Pointer to charinfo to return information on.
  */
GRNG_EXPORT int gr_cinfo_break_weight(const gr_char_info* p/*not NULL*/);

/** Returns the number of unicode characters in a string.
  *
  * @return number of characters in the string
  * @param enc Specifies the type of data in the string: utf8, utf16, utf32
  * @param buffer_begin The start of the string
  * @param buffer_end Measure up to the first nul or when end is reached, whichever is earliest.
  *            This parameter may be NULL.
  * @param pError If there is a structural fault in the string, the location is returned
  *               in this variable. If no error occurs, pError will contain NULL. NULL
  *               may be passed for pError if no such information is required.
  */
GRNG_EXPORT size_t gr_count_unicode_characters(enum gr_encform enc, const void* buffer_begin, const void* buffer_end, const void** pError);

/** Creates and returns a segment.
  *
  * @return a segment that needs seg_destroy called on it.
  * @param font Gives the size of the font in pixels per em for final positioning. If
  *             NULL, positions are returned in design units, i.e. at a ppm of the upem
  *             of the face.
  * @param face The face containing all the non-size dependent information.
  * @param script This is a tag containing a script identifier that is used to choose
  *               which graphite table within the font to use. Maybe 0.
  * @param pFeats Pointer to a feature values to be used for the segment. Only one
  *               feature values may be used for a segment. If NULL the default features
  *               for the font will be used.
  * @param enc Specifies what encoding form the string is in (utf8, utf16, utf32)
  * @param pStart Start of the string
  * @param nChars Number of unicode characters to process in the string
  * @param dir Specifies whether the segment is processed right to left (1) or left to
  *            right (0)
  */
GRNG_EXPORT gr_segment* gr_make_seg(const gr_font* font, const gr_face* face, gr_uint32 script, const gr_feature_val* pFeats, enum gr_encform enc, const void* pStart, size_t nChars, int dir);

/** Destroys a segment, freeing the memory.
  *
  * @param p The segment to destroy
  */
GRNG_EXPORT void gr_seg_destroy(gr_segment* p);

/** Returns the advance for the whole segment.
  *
  * Returns the width of the segment up to the next glyph origin after the segment
  */
GRNG_EXPORT float gr_seg_advance_X(const gr_segment* pSeg/*not NULL*/);

/** Returns the height advance for the segment. **/
GRNG_EXPORT float gr_seg_advance_Y(const gr_segment* pSeg/*not NULL*/);

/** Returns the number of gr_char_infos in the segment. **/
GRNG_EXPORT unsigned int gr_seg_n_cinfo(const gr_segment* pSeg/*not NULL*/);

/** Returns a gr_char_info at a given index in the segment. **/
GRNG_EXPORT const gr_char_info* gr_seg_cinfo(const gr_segment* pSeg/*not NULL*/, unsigned int index/*must be <number_of_CharInfo*/);

/** Returns the number of glyph gr_slots in the segment. **/
GRNG_EXPORT unsigned int gr_seg_n_slots(const gr_segment* pSeg/*not NULL*/);      //one slot per glyph

/** Returns the first gr_slot in the segment.
  *
  * The first slot in a segment has a gr_slot_prev_in_segment() of NULL. Slots are owned
  * by their segment and are destroyed along with the segment.
  */
GRNG_EXPORT const gr_slot* gr_seg_first_slot(gr_segment* pSeg/*not NULL*/);    //may give a base slot or a slot which is attached to another

/** Returns the last gr_slot in the segment.
  *
  * The last slot in a segment has a gr_slot_next_in_segment() of NULL
  */
GRNG_EXPORT const gr_slot* gr_seg_last_slot(gr_segment* pSeg/*not NULL*/);    //may give a base slot or a slot which is attached to another

/** Calculates the underlying character to glyph associations.
  *
  * @param pSeg  Pointer to the segment we want information on.
  * @param begins An array of gr_seg_n_cinfo integers giving slot index for each
  *               charinfo. The value corresponds to which slot a cursor would be before
  *               if an underlying cursor were before the charinfo at this index.
  * @param ends  An array of gr_seg_n_cinfo integers giving the slot index for each
  *              charinfo. The value at an index corresponds to which slot a cursor would
  *              be after if an underlying cursor were after the charinfo at the index.
  * @param sbegins   An array of gr_seg_n_cinfo gr_slot * corresponding to the gr_slot at
  *                  index given by begins. The pointer to the array may be NULL.
  * @param sends An array of gr_seg_n_cinfo gr_slot * corresponding to the gr_slot at the
  *              index given by ends. The pointer to the array may be NULL.
  */
GRNG_EXPORT void gr_seg_char_slots(const gr_segment *pSeg, gr_uint32 *begins, gr_uint32 *ends, gr_slot **sbegins, gr_slot **sends);

/** Returns the next slot along in the segment.
  *
  * Slots are held in a linked list. This returns the next in the linked list. The slot
  * may or may not be attached to another slot. Returns NULL at the end of the segment.
  */
GRNG_EXPORT const gr_slot* gr_slot_next_in_segment(const gr_slot* p);

/** Returns the previous slot along in the segment.
  *
  * Slots are held in a doubly linked list. This returns the previos slot in the linked
  * list. This slot may or may not be attached to it. Returns NULL at the start of the
  * segment.
  */
GRNG_EXPORT const gr_slot* gr_slot_prev_in_segment(const gr_slot* p);

/** Returns the attachment parent slot of this slot.
  *
  * Attached slots form a tree. This returns the parent of this slot in that tree. A
  * base glyph which is not attached to another glyph, always returns NULL.
  */
GRNG_EXPORT const gr_slot* gr_slot_attached_to(const gr_slot* p);

/** Returns the first slot attached to this slot.
  *
  * Attached slots form a singly linked list from the parent. This returns the first
  * slot in that list. Note that this is a reference to another slot that is also in
  * the main segment doubly linked list.
  *
  * if gr_slot_first_attachment(p) != NULL then gr_slot_attached_to(gr_slot_first_attachment(p)) == p.
  */
GRNG_EXPORT const gr_slot* gr_slot_first_attachment(const gr_slot* p);

/** Returns the next slot attached to our attachment parent.
  *
  * This returns the next slot in the singly linked list of slots attached to this
  * slot's parent. If there are no more such slots, NULL is returned. If there is no parent, i.e.
  * the passed slot is a base, then the next base in segment order is returned.
  *
  * if gr_slot_next_sibling_attachment(p) != NULL then gr_slot_attached_to(gr_slot_next_sibling_attachment(p)) == gr_slot_attached_to(p).
  */
GRNG_EXPORT const gr_slot* gr_slot_next_sibling_attachment(const gr_slot* p);


/** Returns glyph id of the slot
  *
  * Each slot has a glyphid which is rendered at the position given by the slot. This
  * glyphid is the real glyph to be rendered and never a pseudo glyph.
  */
GRNG_EXPORT unsigned short gr_slot_gid(const gr_slot* p);

/** Returns X offset of glyph from start of segment **/
GRNG_EXPORT float gr_slot_origin_X(const gr_slot* p);

/** Returns Y offset of glyph from start of segment **/
GRNG_EXPORT float gr_slot_origin_Y(const gr_slot* p);

/** Returns the glyph advance for this glyph as adjusted for kerning **/
GRNG_EXPORT float gr_slot_advance(const gr_slot* p);

/** Returns the gr_char_info index before us
  *
  * Returns the index of the gr_char_info that a cursor before this slot, would put
  * an underlying cursor before.
  */
GRNG_EXPORT int gr_slot_before(const gr_slot* p/*not NULL*/);

/** Returns the gr_char_info index after us
  *
  * Returns the index of the gr_char_info that a cursor after this slot would put an
  * underlying cursor after.
  */
GRNG_EXPORT int gr_slot_after(const gr_slot* p/*not NULL*/);

/** Return a slot attribute value
  *
  * Given a slot and an attribute along with a possible subattribute, return the
  * corresponding value in the slot. See enum gr_attrCode for details of each attribute.
  */
GRNG_EXPORT int gr_slot_attr(const gr_slot* p/*not NULL*/, const gr_segment* pSeg/*not NULL*/, enum gr_attrCode index, gr_uint8 subindex); //tbd - do we need to expose this?

/** Returns whether text may be inserted before this glyph [check this isn't inverted] **/
GRNG_EXPORT int gr_slot_can_insert_before(const gr_slot* p);

/** Returns the original gr_char_info index this slot refers to.
  *
  * Each Slot has a gr_char_info that it originates from. This is that gr_char_info. This
  * information is useful for testing.
  */
GRNG_EXPORT int gr_slot_original(const gr_slot* p/*not NULL*/);
  
#ifdef __cplusplus
}
#endif
