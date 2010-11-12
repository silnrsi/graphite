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

namespace org { namespace sil { namespace graphite { namespace v2 {

class GrFont;
class Slot;
class GrSegment;

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


extern "C"
{
    //slots are owned by their segment
    GRNG_EXPORT const Slot* next_slot_in_segment(const Slot* p/*not NULL*/);
    GRNG_EXPORT unsigned short gid(const Slot* p/*not NULL*/);
    GRNG_EXPORT float origin_X(const Slot* p/*not NULL*/);
    GRNG_EXPORT float origin_Y(const Slot* p/*not NULL*/);
    GRNG_EXPORT float advance(const Slot* p/*not NULL*/, const GrFont *font);
    GRNG_EXPORT int before(const Slot* p/*not NULL*/);
    GRNG_EXPORT int after(const Slot* p/*not NULL*/);
    GRNG_EXPORT int get_attr(const Slot* p/*not NULL*/, const GrSegment* pSeg/*not NULL*/, attrCode index, uint8 subindex);
    GRNG_EXPORT bool is_insert_before(const Slot* p/*not NULL*/);
    GRNG_EXPORT int original(const Slot* p/*not NULL*/);
//  GRNG_EXPORT size_t id(const Slot* p/*not NULL*/);
    GRNG_EXPORT size_t attached_to(const Slot* p/*not NULL*/);
}

}}}} // namespace
