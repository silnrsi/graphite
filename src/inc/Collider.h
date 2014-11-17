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

Alternatively, the contents of this file may be used under the terms of the
Mozilla Public License (http://mozilla.org/MPL) or the GNU General Public
License, as published by the Free Software Foundation, either version 2
of the License or (at your option) any later version.
*/
#pragma once

#include <utility>
#include "inc/List.h"
#include "inc/Slot.h"
#include "inc/Position.h"
#include "inc/Intervals.h"
#include "inc/debug.h"
#include "inc/Segment.h"

namespace graphite2 {

class json;
class Slot;

class Collider
{
public:
    typedef std::pair<float, float> fpair;
    typedef Vector<fpair> vfpairs;
    typedef vfpairs::iterator ivfpairs;

    virtual ~Collider() throw() { };
    virtual void initSlot(GR_MAYBE_UNUSED Segment *seg, GR_MAYBE_UNUSED Slot *aSlot, GR_MAYBE_UNUSED const Rect &constraint,
                GR_MAYBE_UNUSED float margin, GR_MAYBE_UNUSED const Position &currShift, GR_MAYBE_UNUSED float currKern,
                GR_MAYBE_UNUSED int dir, GR_MAYBE_UNUSED json * const dbgout)
        { };
    virtual bool mergeSlot(GR_MAYBE_UNUSED Segment *seg, GR_MAYBE_UNUSED Slot *slot,
                GR_MAYBE_UNUSED const Position &currShift, GR_MAYBE_UNUSED const float currKern, GR_MAYBE_UNUSED bool ignoreForKern,
                GR_MAYBE_UNUSED json * const dbgout)
        { return false; }
    virtual Position resolve(GR_MAYBE_UNUSED Segment *seg, GR_MAYBE_UNUSED bool &isCol, GR_MAYBE_UNUSED json * const dbgout)
        { return Position(); }
        
#if !defined GRAPHITE2_NTRACING
    void debug(json * const dbgout, Segment *seg, int i) {
//        if (!dbgout) return;
        int imax = i;
        if (i < 0)
        {
            *dbgout << "margin" << _margin
                << "limit" << _limit
                << "target" << json::object
                    << "origin" << _target->origin()
                    << "bbox" << seg->theGlyphBBoxTemporary(_target->gid())
                    << "slantbox" << seg->getFace()->glyphs().slant(_target->gid())
                    << json::close; // target object
            *dbgout << "ranges" << json::array;
            i = 0;
            imax = 3;
        }
        for (int j = i; j <= imax; ++j)
        {
            *dbgout << json::flat << json::array;
            for (IntervalSet::ivtpair s = _ranges[j].begin(), e = _ranges[j].end(); s != e; ++s)
                *dbgout << Position(s->first, s->second);
            *dbgout << json::close;
        }
        if (i < imax) // looped through the _ranges array
            *dbgout << json::close; // ranges array
    }
#endif

    CLASS_NEW_DELETE;

protected:
    IntervalSet _ranges[4]; // possible movements in 4 directions (horizontally, vertically, diagonally);
                            // for KernColliders these are 4 horizontal strata across the target glyph
    Slot *  _target;        // the glyph to fix
    Rect    _limit;
    float   _margin;
    Position _currShift;
    float   _kern;          // kerning that has happened in previous glyphs
    
    // Debugging
    IntervalSet _rawRanges[4];
    IntervalSet _removals[4];
    Vector<int> _gidNear[4];
    Vector<int> _subNear[4];
    Vector<int> _subTarget[4];

};

class ShiftCollider : public Collider
{
public:
    virtual ~ShiftCollider() throw() { };
    virtual void initSlot(Segment *seg, Slot *aSlot, const Rect &constraint, float margin, const Position &currShift, 
                float currKern, int dir, json * const dbgout);
    virtual bool mergeSlot(Segment *seg, Slot *slot, const Position &currShift, const float currKern, bool fIgnoreForKern,
                json * const dbgout);
    virtual Position resolve(Segment *seg, bool &isCol, json * const dbgout);

    CLASS_NEW_DELETE;

};

class KernCollider : public Collider
{
public:
    virtual ~KernCollider() throw() { };
    virtual void initSlot(Segment *seg, Slot *aSlot, const Rect &constraint, float margin, const Position &currShift, 
                float currKern, int dir, json * const dbgout);
    virtual bool mergeSlot(Segment *seg, Slot *slot, const Position &currShift, const float currKern, bool fIgnoreForKern,
                json * const dbgout);
    virtual Position resolve(Segment *seg, bool &isCol, json * const dbgout);

    CLASS_NEW_DELETE;

private:
    bool removeXCovering(uint16 gid, uint16 tgid, const GlyphCache &gc, float sx, float sy, float tx, float ty,
    		int it, int ig, IntervalSet &range,
    		int row);
    Rect _limitSpec;  // limits specified, as opposed to practical limits for determining possible movement
    float _miny;	  // y-coordinates offset by global slot position
    float _maxy;
};


class SlotCollision
{
public:
    enum {
        COLL_TEST = 1,
        COLL_IGNORE = 2,
        COLL_START = 4,
        COLL_END = 8,
        COLL_KERN = 16,
        COLL_ISCOL = 32,
        COLL_KNOWN = 64,
    };
        
    SlotCollision(Segment *seg, Slot *slot);
    const Rect &limit() const { return _limit; }
    void limit(const Rect &r) { _limit = r; }
    const Position &shift() const { return _shift; }
    void shift(const Position &s) { _shift = s; }
    uint16 margin() const { return _margin; }
    void margin(uint16 m) { _margin = m; }
    uint16 flags() const { return _flags; }
    void flags(uint16 f) { _flags = f; }
    
    float getKern(int dir) const;
    
private:
    Rect        _limit;
    Position    _shift;
    uint16      _margin;
    uint16      _flags;
};

};
