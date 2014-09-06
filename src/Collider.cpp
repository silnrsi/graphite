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
#include <algorithm>
#include <limits>
#include <math.h>
#include "inc/Collider.h"
#include "inc/Segment.h"

#define ISQRT2 0.707106781f

using namespace graphite2;

void BoundedGapList::reset(float min, float max)
{
    _min = min;
    _max = max;
    _list.assign(1, fpair(_min, _max));
    _isLenSorted = false;
}

static bool _byfirst(const BoundedGapList::fpair &a, const BoundedGapList::fpair &b)
{
    return (a.first < b.first);
}

static bool _bylen(const BoundedGapList::fpair &a, const BoundedGapList::fpair &b)
{
    return (a.second - a.first < b.second - b.first ||
            (a.second - a.first == b.second - b.first && a.first < b.first));
}

void BoundedGapList::add(float min, float max)
{
    ivfpairs kind, kend;

    if (max < _min || min > _max) return;

    if (_isLenSorted)
    { 
        std::sort(_list.begin(), _list.end(), _byfirst);
        _isLenSorted = false;
    }

    // need code here and above to keep the n best pairs rather than all of them.
    for (kind = _list.begin(), kend = _list.end(); kind != kend; ++kind)
    {    
        if (kind->second < min) continue;
        else if (kind->first > max) 
        {
            _list.insert(kind, fpair(max, kind->first));
            if (_isLenSorted) continue;
            else break;
        }
        else if (kind->first > min && kind->second < max)
        {
            _list.erase(kind);
            --kind;
        }
        if (kind->first < min && kind->second > max) // need to split and add a range
        {
            _list.insert(kind, fpair(kind->first, min));
            kind->first = max;
            if (!_isLenSorted) break;
        }
        else if (kind->second < min)
            kind->second = min;
        else
            kind->first = max;
    }
}

static bool _cmplen(const BoundedGapList::fpair &t, const BoundedGapList::fpair &a)
{
    return (t.second - t.first < a.second - a.first);
}

float BoundedGapList::bestfit(float min, float max, bool &isGapFit)
{
    ivfpairs kstart, kend = _list.end();
    float res = std::numeric_limits<float>::max();

    if (!_isLenSorted)
    {
        std::sort(_list.begin(), _list.end(), _bylen);
        _isLenSorted = true;
    }
    
    kstart = std::upper_bound(_list.begin(), _list.end(), fpair(min, max), _cmplen);
    if (kstart != kend)
    {
        isGapFit = true;
        for ( ; kstart != kend; ++kstart)
        {
            float ires = kstart->first - min;
            if (ires < 0.f && kstart->second > max)  // no collision, return no movement
                return 0.f;
            else if (ires < 0.f)
                ires = kstart->second - max;
            if (fabs(res) > fabs(ires))
                res = ires;
        }
    }
    else
    {
        ivfpairs kind = --kstart;
        isGapFit = false;
        kend = _list.begin();
        --kend;
        for ( ; kind != kend; --kind)
        {
            if (kind->second - kind->first != kstart->second - kstart->first)
                break;
            //float ires = 0.5 * (max - min) - 0.5 * (kind->second - kind->first) + min;
            float ires = 0.5 * (max - min - kind->second + kind->first);
            if (fabs(ires) < fabs(res))
                res = ires;
        }
    }
    return res;
}


void Collider::initSlot(Slot *aSlot, const Rect &limit)
{
    int i;
    float max, min;
    for (i = 0; i < 4; ++i)
    {
        switch (i) {
            case 0 :
                min = limit.bl.x;
                max = limit.tr.x;
                break;
            case 1 :
                min = limit.bl.y;
                max = limit.tr.y;
                break;
            case 2 :
                min = 2.f * std::max(limit.bl.x, -limit.tr.y);
                max = 2.f * std::min(limit.tr.x, -limit.bl.y);
                break;
            case 3 :
                min = 2.f * std::max(limit.bl.x, limit.bl.y);
                max = 2.f * std::min(limit.tr.x, limit.tr.y);
                break;
        } 
        _ranges[i].reset(min, max);
    }
    _base = aSlot;
    _limit = limit;
}

bool cmpfpair(float a, const Collider::fpair &b)
{
    return a < b.first;
}

bool Collider::mergeSlot(Segment *seg, Slot *slot)
{
    bool isCol = true;
    const float bx = _base->origin().x;
    const float by = _base->origin().y;
    const float bd = bx - by;
    const float bs = bx + by;
    const float sx = slot->origin().x;
    const float sy = slot->origin().y;
    const float sd = sx - sy;
    const float ss = sx + sy;
    float vmin, vmax;
    float omin, omax, obmin, obmax;
    float cmin, cmax;
    const GlyphCache &gc = seg->getFace()->glyphs();
    const unsigned short gid = slot->gid();
    const unsigned short bgid = _base->gid();
    const uint16 m = seg->collisionInfo(_base)->margin();
    for (int i = 0; i < 4; ++i)
    {
        switch (i) {
            case 0 :
                vmin = std::max(std::max(gc.getBoundingMetric(gid, 0) + sx,
                                gc.getBoundingMetric(gid, 5) + sd + gc.getBoundingMetric(bgid, 1) + by),
                                gc.getBoundingMetric(gid, 4) + ss - gc.getBoundingMetric(bgid, 3) - by);
                vmax = std::min(std::min(gc.getBoundingMetric(gid, 2) + sx,
                                gc.getBoundingMetric(gid, 7) + sd + gc.getBoundingMetric(bgid, 3) + by),
                                gc.getBoundingMetric(gid, 6) + ss - gc.getBoundingMetric(bgid, 1) - by);
                obmin = gc.getBoundingMetric(bgid, 1) + bx;
                obmax = gc.getBoundingMetric(bgid, 3) + bx;
                omin = gc.getBoundingMetric(gid, 1) + sy;
                omax = gc.getBoundingMetric(gid, 3) + sy;
                cmin = _limit.bl.x + bx;
                cmax = _limit.tr.x + bx;
                break;
            case 1 :
                vmin = std::max(std::max(gc.getBoundingMetric(gid, 1) + sy,
                                gc.getBoundingMetric(bgid, 0) + bx - gc.getBoundingMetric(gid, 7) - sd),
                                gc.getBoundingMetric(gid, 4) + ss - gc.getBoundingMetric(bgid, 2) - bx);
                vmax = std::min(std::min(gc.getBoundingMetric(gid, 3) + sy,
                                gc.getBoundingMetric(bgid, 2) + bx - gc.getBoundingMetric(gid, 5) - sd),
                                gc.getBoundingMetric(gid, 6) + ss - gc.getBoundingMetric(bgid, 0) - bx);
                obmin = gc.getBoundingMetric(bgid, 0) + bx;
                obmax = gc.getBoundingMetric(bgid, 2) + bx;
                omin = gc.getBoundingMetric(gid, 0) + sx;
                omax = gc.getBoundingMetric(gid, 2) + sx;
                cmin = _limit.bl.y + by;
                cmax = _limit.tr.y + by;
                break;
            case 2 :    // s
                vmin = std::max(std::max(gc.getBoundingMetric(gid, 4) + ss,
                                2 * gc.getBoundingMetric(gid, 1) + 2 * sy + gc.getBoundingMetric(bgid, 5) + bd),
                                2 * gc.getBoundingMetric(gid, 0) + 2 * sx - gc.getBoundingMetric(bgid, 7) - bd);
                vmax = std::min(std::min(gc.getBoundingMetric(gid, 6) + ss,
                                2 * gc.getBoundingMetric(gid, 3) + 2 * sy + gc.getBoundingMetric(bgid, 7) + bd),
                                2 * gc.getBoundingMetric(gid, 2) + 2 * sx - gc.getBoundingMetric(bgid, 5) - bd);
                obmin = gc.getBoundingMetric(bgid, 5) + bd;
                obmax = gc.getBoundingMetric(bgid, 7) + bd;
                omin = gc.getBoundingMetric(gid, 5) + sd;
                omax = gc.getBoundingMetric(gid, 7) + sd;
                cmin = _limit.bl.x + _limit.bl.y + bs;
                cmax = _limit.tr.x + _limit.tr.y + bs;
                break;
            case 3 :    // d
                vmin = std::max(std::max(gc.getBoundingMetric(gid, 5) + sd,
                                2 * gc.getBoundingMetric(gid, 0) + 2 * sx - gc.getBoundingMetric(bgid, 6) - bs),
                                gc.getBoundingMetric(bgid, 4) + bs - 2 * gc.getBoundingMetric(gid, 3) - 2 * sy);
                vmax = std::min(std::min(gc.getBoundingMetric(gid, 7) + sd,
                                2 * gc.getBoundingMetric(gid, 2) + 2 * sx - gc.getBoundingMetric(bgid, 4) - bs),
                                gc.getBoundingMetric(bgid, 6) + bs - 2 * gc.getBoundingMetric(gid, 1) - 2 * sy);
                obmin = gc.getBoundingMetric(bgid, 4) + bs;
                obmax = gc.getBoundingMetric(bgid, 6) + bs;
                omin = gc.getBoundingMetric(gid, 4) + ss;
                omax = gc.getBoundingMetric(gid, 6) + ss;
                cmin = _limit.bl.x - _limit.tr.y + bd;
                cmax = _limit.tr.x - _limit.bl.y + bd;
                break;
            default :
                continue;
        }
        if ((vmin < cmin && vmax < cmin) || (vmin > cmax && vmax > cmax)
            || (omin < obmin - m && omax < obmin - m) || (omin > obmax + m && omax > obmax + m))
        {
            isCol = false;
            continue;
        }
        uint8 numsub = gc.numSubBounds(gid);
        if (numsub > 0)
        {
            bool anyhits = false;
            for (int j = 0; j < numsub; ++j)
            {
                switch (i) {
                    case 0 :    // x
                        vmin = std::max(std::max(gc.getSubBoundingMetric(gid, j, 0) + sx,
                                        gc.getSubBoundingMetric(gid, j, 5) + sd + gc.getBoundingMetric(bgid, 1) + by),
                                        gc.getSubBoundingMetric(gid, j, 4) + ss - gc.getBoundingMetric(bgid, 3) - by);
                        vmax = std::min(std::min(gc.getSubBoundingMetric(gid, j, 2) + sx,
                                        gc.getSubBoundingMetric(gid, j, 7) + sd + gc.getBoundingMetric(bgid, 3) + by),
                                        gc.getSubBoundingMetric(gid, j, 6) + ss - gc.getBoundingMetric(bgid, 1) - by);
                        omin = gc.getSubBoundingMetric(gid, j, 1) + sy;
                        omax = gc.getSubBoundingMetric(gid, j, 3) + sy;
                        break;
                    case 1 :    // y
                        vmin = std::max(std::max(gc.getSubBoundingMetric(gid, j, 1) + sy,
                                        gc.getBoundingMetric(bgid, 0) + bx - gc.getSubBoundingMetric(gid, j, 7) - sd),
                                        gc.getSubBoundingMetric(gid, j, 4) + ss - gc.getBoundingMetric(bgid, 2) - bx);
                        vmax = std::min(std::min(gc.getSubBoundingMetric(gid, j, 3) + sy,
                                        gc.getBoundingMetric(bgid, 2) + bx - gc.getSubBoundingMetric(gid, j, 5) - sd),
                                        gc.getSubBoundingMetric(gid, j, 6) + ss - gc.getBoundingMetric(bgid, 0) - bx);
                        omin = gc.getSubBoundingMetric(gid, j, 0) + sx;
                        omax = gc.getSubBoundingMetric(gid, j, 2) + sx;
                        break;
                    case 2 :    // s
                        vmin = std::max(std::max(gc.getSubBoundingMetric(gid, j, 4) + ss,
                                        2 * gc.getSubBoundingMetric(gid, j, 1) + 2 * sy + gc.getBoundingMetric(bgid, 5) + bd),
                                        2 * gc.getSubBoundingMetric(gid, j, 0) + 2 * sx - gc.getBoundingMetric(bgid, 7) - bd);
                        vmax = std::min(std::min(gc.getSubBoundingMetric(gid, j, 6) + ss,
                                        2 * gc.getSubBoundingMetric(gid, j, 3) + 2 * sy + gc.getBoundingMetric(bgid, 7) + bd),
                                        2 * gc.getSubBoundingMetric(gid, j, 2) + 2 * sx - gc.getBoundingMetric(bgid, 5) - bd);
                        omin = gc.getSubBoundingMetric(gid, j, 5) + sd;
                        omax = gc.getSubBoundingMetric(gid, j, 7) + sd;
                        break;
                    case 3 :    // d
                        vmin = std::max(std::max(gc.getSubBoundingMetric(gid, j, 5) + sd,
                                        2 * gc.getSubBoundingMetric(gid, j, 0) + 2 * sx - gc.getBoundingMetric(bgid, 6) - bs),
                                        gc.getBoundingMetric(bgid, 4) + bs - 2 * gc.getSubBoundingMetric(gid, j, 3) - 2 * sy);
                        vmax = std::min(std::min(gc.getSubBoundingMetric(gid, j, 7) + sd,
                                        2 * gc.getSubBoundingMetric(gid, j, 2) + 2 * sx - gc.getBoundingMetric(bgid, 4) - bs),
                                        gc.getBoundingMetric(bgid, 6) + bs - 2 * gc.getSubBoundingMetric(gid, j, 1) - 2 * sy);
                        omin = gc.getSubBoundingMetric(gid, j, 4) + ss;
                        omax = gc.getSubBoundingMetric(gid, j, 6) + ss;
                        break;
                }
                if ((vmin < cmin && vmax < cmin) || (vmin > cmax && vmax > cmax)
                    || (omin < obmin - m && omax < obmin - m) || (omin > obmax + m && omax > obmax + m))
                    continue;
                _ranges[i].add(vmin, vmax);
                anyhits = true;
            }
            if (!anyhits)
                isCol = false;
        }
        else
            _ranges[i].add(vmin, vmax);
    }
    return isCol;
}


Position Collider::resolve(Segment *seg, bool &isCol, const Position &currshift)
{
    const GlyphCache &gc = seg->getFace()->glyphs();
    int gid = _base->gid();
    float margin;
    float tlen, torig;
    float totald = std::numeric_limits<float>::max();
    Position totalp;
    // float cmax, cmin;
    bool isGoodFit, tIsGoodFit = false;
    for (int i = 0; i < 4; ++i)
    {
        float bestc = std::numeric_limits<float>::max();
        float bestd = bestc;
        margin = seg->collisionInfo(_base)->margin() * (i > 1 ? ISQRT2 : 1.f);
        switch (i) {
            case 0 :
                tlen = gc.getBoundingMetric(gid, 2) - gc.getBoundingMetric(gid, 0);
                torig = _base->origin().x + currshift.x + gc.getBoundingMetric(gid, 0);
                // cmin = _limit.bl.x + torig;
                // cmax = _limit.tr.x + torig;
                break;
            case 1 :
                tlen = gc.getBoundingMetric(gid, 3) - gc.getBoundingMetric(gid, 1);
                torig = _base->origin().y + currshift.y + gc.getBoundingMetric(gid, 1);
                // cmin = _limit.bl.y + torig;
                // cmax = _limit.tr.y + torig;
                break;
            case 2 :
                tlen = gc.getBoundingMetric(gid, 6) - gc.getBoundingMetric(gid, 4);
                torig = _base->origin().x + _base->origin().y + currshift.x + currshift.y + gc.getBoundingMetric(gid, 4);
                // cmin = std::max(_limit.bl.x, _limit.bl.y) + torig;
                // cmax = std::min(_limit.tr.x, _limit.tr.y) + torig;
                break;
            case 3 :
                tlen = gc.getBoundingMetric(gid, 7) - gc.getBoundingMetric(gid, 5);
                torig = _base->origin().x - _base->origin().y + currshift.x - currshift.y + gc.getBoundingMetric(gid, 5);
                // cmin = std::max(_limit.bl.x, -_limit.tr.y) + torig;
                // cmax = std::min(_limit.tr.x, -_limit.bl.y) + torig;
                break;
        }
        bestd = _ranges[i].bestfit(torig - margin, torig + tlen + margin, isGoodFit);
        bestd += bestd > 0.f ? -margin : margin;
        if ((isGoodFit && !tIsGoodFit) || fabs(bestd) * (i > 1 ? ISQRT2 : 1.f) < totald)
        {
            totald = fabs(bestd) * (i > 1 ? ISQRT2 : 1.);
            tIsGoodFit = isGoodFit;
            switch (i) {
                case 0 : totalp = Position(bestd, 0.); break;
                case 1 : totalp = Position(0., bestd); break;
                case 2 : totalp = Position(bestd, bestd); break;
                case 3 : totalp = Position(bestd, -bestd); break;
            }
        }
    }
    isCol = !tIsGoodFit;
    return totalp;
}

SlotCollision::SlotCollision(Segment *seg, Slot *slot)
{
    uint16 gid = slot->gid();
    uint16 aCol = seg->silf()->aCollision();
    _limit = Rect(Position(seg->glyphAttr(gid, aCol+1) + slot->origin().x,
                           seg->glyphAttr(gid, aCol+2) + slot->origin().y),
                  Position(seg->glyphAttr(gid, aCol+3) + slot->origin().x,
                           seg->glyphAttr(gid, aCol+4) + slot->origin().y));
    _margin = seg->glyphAttr(gid, aCol+5);
    _flags = seg->glyphAttr(gid, aCol);
}

