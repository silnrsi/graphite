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

#include "graphiteng/Types.h"
#include "graphiteng/Font.h"

#ifdef __cplusplus
namespace org { namespace sil { namespace graphite { namespace v2 {
extern "C"
{
#endif

enum {
    /* after break weights */
    eBreakWhitespace = 10,
    eBreakWord = 15,
    eBreakIntra = 20,
    eBreakLetter = 30,
    eBreakClip = 40,
    /* before break weights */
    eBreakBeforeWhitespace = -10,
    eBreakBeforeWord = -15,
    eBreakBeforeIntra = -20,
    eBreakBeforeLetter = -30,
    eBreakBeforeClip = -40
};

enum attrCode {
    kslatAdvX = 0, kslatAdvY,
    kslatAttTo,
    kslatAttX, kslatAttY, kslatAttGpt,
    kslatAttXOff, kslatAttYOff,
    kslatAttWithX, kslatAttWithY, kslatWithGpt,
    kslatAttWithXOff, kslatAttWithYOff,
    kslatAttLevel,
    kslatBreak,
    kslatCompRef,
    kslatDir,
    kslatInsert,
    kslatPosX, kslatPosY,
    kslatShiftX, kslatShiftY,
    kslatUserDefnV1,
    kslatMeasureSol, kslatMeasureEol,
    kslatJStretch, kslatJShrink, kslatJStep, kslatJWeight, kslatJWidth,
    
    kslatUserDefn = kslatJStretch + 30,
    
    kslatMax,
    kslatNoEffect = kslatMax + 1
};


typedef struct CharInfo GrCharInfo;
typedef struct GrSegment GrSegment;
typedef struct Slot GrSlot;

    /** Returns Unicode character for a charinfo.
     * 
     * @param p Pointer to charinfo to return information on.
     */
    GRNG_EXPORT unsigned int cinfo_unicode_char(const GrCharInfo* p/*not NULL*/);
    
    /** Returns breakweight for a charinfo.
     * 
     * @return Breakweight is a number between -50 and 50 indicating the cost of a
     * break before or after this character.
     * @param p Pointer to charinfo to return information on.
     */
    GRNG_EXPORT int cinfo_break_weight(const GrCharInfo* p/*not NULL*/);

    /** Returns the number of unicode characters in a string
     *
     * @return number of characters in the string
     * @param enc Specifies the type of data in the string: utf8, utf16, utf32
     * @param begin The start of the string
     * @param end Measure up to the first nul or when end is reached, whichever is earliest.
     *            This parameter may be NULL.
     * @param pError If there is a structural fault in the string, the location is returned
     *               in this variable. If no error occurs, pError will contain NULL. NULL
     *               may be passed for pError if no such information is required.
     */
    GRNG_EXPORT size_t count_unicode_characters(enum encform enc, const void* buffer_begin, const void* buffer_end, const void** pError);

    /** Creates and returns a segment
     *
     * @return a segment that needs seg_destroy called on it.
     * @param font Gives the size of the font in pixels per em for final positioning. If
     *             NULL, positions are returned in design units, i.e. at a ppm of the upem
     *             of the face.
     * @param face The face containing all the non-size dependent information.
     * @param script This is a tag containing a script identifier that is used to choose
     *               which graphite table within the font to use. Maybe 0.
     * @param pFests Pointer to a feature values to be used for the segment. Only one
     *               feature values may be used for a segment. If NULL the default features
     *               for the font will be used.
     * @param enc Specifies what encoding form the string is in (utf8, utf16, utf32)
     * @param pStart Start of the string
     * @param nChars Number of unicode characters to process in the string
     * @param dir Specifies whether the segment is processed right to left (1) or left to
     *            right (0)
     */
    GRNG_EXPORT GrSegment* make_seg(const GrFont* font, const GrFace* face, uint32 script, const Features* pFeats, enum encform enc, const void* pStart, size_t nChars, int dir);

    /** Destroys a segment, freeing the memory
     *
     * @param p The segment to destroy
     */
    GRNG_EXPORT void seg_destroy(GrSegment* p);

    GRNG_EXPORT float seg_advance_X(const GrSegment* pSeg/*not NULL*/);
    GRNG_EXPORT float seg_advance_Y(const GrSegment* pSeg/*not NULL*/);
    GRNG_EXPORT unsigned int seg_n_cinfo(const GrSegment* pSeg/*not NULL*/);
    GRNG_EXPORT const GrCharInfo* seg_cinfo(const GrSegment* pSeg/*not NULL*/, unsigned int index/*must be <number_of_CharInfo*/);
    GRNG_EXPORT unsigned int seg_n_slots(const GrSegment* pSeg/*not NULL*/);      //one slot per glyph
    GRNG_EXPORT const GrSlot* seg_first_slot(GrSegment* pSeg/*not NULL*/);    //may give a base slot or a slot which is attached to another
    GRNG_EXPORT void seg_char_slots(const GrSegment *pSeg, uint32 *begins, uint32 *ends, GrSlot **sbegins, GrSlot **sends);
    //slots are owned by their segment
    GRNG_EXPORT const GrSlot* slot_next_in_segment(const GrSlot* p/*not NULL*/);    //may give a base slot or a slot which is attached to another
    GRNG_EXPORT const GrSlot* slot_attached_to(const GrSlot* p/*not NULL*/);        //returns NULL iff base. If called repeatedly on result, will get to a base
 
    GRNG_EXPORT const GrSlot* slot_first_attachment(const GrSlot* p/*not NULL*/);        //returns NULL iff no attachments.
        //if slot_first_attachment(p) is not NULL, then slot_attached_to(slot_first_attachment(p))==p.
    
    GRNG_EXPORT const GrSlot* slot_next_sibling_attachment(const GrSlot* p/*not NULL*/);        //returns NULL iff no more attachments.
        //if slot_next_sibling_attachment(p) is not NULL, then slot_attached_to(slot_next_sibling_attachment(p))==slot_attached_to(p).
    
    
    GRNG_EXPORT unsigned short slot_gid(const GrSlot* p/*not NULL*/);
    GRNG_EXPORT float slot_origin_X(const GrSlot* p/*not NULL*/);
    GRNG_EXPORT float slot_origin_Y(const GrSlot* p/*not NULL*/);
    GRNG_EXPORT float slot_advance(const GrSlot* p/*not NULL*/);
    GRNG_EXPORT int slot_before(const GrSlot* p/*not NULL*/);
    GRNG_EXPORT int slot_after(const GrSlot* p/*not NULL*/);
    GRNG_EXPORT int slot_attr(const GrSlot* p/*not NULL*/, const GrSegment* pSeg/*not NULL*/, enum attrCode index, uint8 subindex); //tbd - do we need to expose this?
     
    GRNG_EXPORT int slot_can_insert_before(const GrSlot* p/*not NULL*/);
    GRNG_EXPORT int slot_original(const GrSlot* p/*not NULL*/);
//  GRNG_EXPORT size_t id(const GrSlot* p/*not NULL*/);
  

#ifdef __cplusplus
}
}}}}
#endif
