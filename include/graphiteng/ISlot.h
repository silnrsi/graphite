#ifndef ISLOT_INCLUDE
#define ISLOT_INCLUDE

#include "graphiteng/Types.h"

class LoadedFont;
class SegmentHandle;

enum attrCode {
    kslatAdvX = 0, kslatAdvY,
    kslatAttTo,
    kslatAttX, kslatAttY,
    kslatAttXOff, kslatAttYOff,
    kslatAttWithX, kslatAttWithY,
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
    
class ISlot
{
public:
    virtual unsigned short gid() const = 0;
    virtual Position origin() const = 0;
    virtual float advance(const LoadedFont *font) = 0;
    virtual int before() const = 0;
    virtual int after() const = 0;
    virtual int getAttr(SegmentHandle *seg, uint8 index, uint8 subindex) = 0;
};

#endif // SLOT_INCLUDE

