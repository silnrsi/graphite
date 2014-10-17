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

class BoundedGapList
{
public:
    typedef std::pair<float, float> fpair;
    typedef Vector<fpair> vfpairs;
    typedef vfpairs::iterator ivfpairs;

    void reset(float min, float max);
    void add(float min, float max);
    float bestfit(float min, float max, bool &isGapFit);

private:
    float _min;
    float _max;
    vfpairs _list;
    bool _isLenSorted;
};

class Collider
{
public:
    typedef std::pair<float, float> fpair;
    typedef Vector<fpair> vfpairs;
    typedef vfpairs::iterator ivfpairs;

    void initSlot(Slot *aSlot, const Rect &constraint, float margin);
    bool mergeSlot(Segment *seg, Slot *slot);
    Position resolve(Segment *seg, bool &isCol, const Position &currshift, json * const dbgout);
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
                    << json::close
                << "ranges" << json::array;
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
        if (i < imax)
            *dbgout << json::close;
    }
#endif

private:
    IntervalSet _ranges[4]; // possible movements in 4 directions (horizontally, vertically, diagonally);
    Slot *  _target;        // the glyph to fix
    Rect    _limit;
    float   _margin;
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

private:
    Rect        _limit;
    Position    _shift;
    uint16      _margin;
    uint16      _flags;
};

};
