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
    GRNG_EXPORT int get_attr(const Slot* p/*not NULL*/, const GrSegment* pSeg/*not NULL*/, attrCode index, uint8 subindex); //tbd - do we need to expose this?
    //tbd (07:42:59) ks: the internal isBase, attachedTo, child, sibling need exposing
    
    GRNG_EXPORT bool is_insert_before(const Slot* p/*not NULL*/);
    GRNG_EXPORT int original(const Slot* p/*not NULL*/);
//  GRNG_EXPORT size_t id(const Slot* p/*not NULL*/);
    GRNG_EXPORT size_t attached_to(const Slot* p/*not NULL*/);
}

}}}} // namespace
