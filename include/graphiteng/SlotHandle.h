#pragma once

#include "graphiteng/Types.h"

class GrFont;
class SegmentHandle;
class Slot;

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
    
class GRNG_EXPORT SlotHandle		//stays valid so long as its SegmentHandle stays valid
{
public:
    SlotHandle(const Slot* p/*no ownership, caller must keep it alive*/=NULL) : m_p(p) {}
 
    unsigned short gid() const;
    float originX() const;
    float originY() const;
    float advance(const GrFont *font) const;
    int before() const;
    int after() const;
    int getAttr(const SegmentHandle& hSeg, attrCode index, uint8 subindex, int is) const;
    bool isInsertBefore() const;
    int original() const;

    const Slot* operator->() const { return m_p; }		//cannot be used by client code - only available witin graphite code!

protected:
//    const Slot& operator*() const {return *m_p; };
    const Slot* ptr() const { return m_p; }

    void setPtr(const Slot* p/*no ownership, caller must keep it alive*/=NULL) { m_p=p; }
  

private:
    const Slot* m_p;		//not owned
};


