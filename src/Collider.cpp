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
#include "inc/Slot.h"

#define ISQRT2 0.707106781f

// Possible rounding error for subbox boundaries: 0.016 = 1/64 = 1/256 * 4 
// (values in font range from 0..256)
#define SUBBOX_RND_ERR 0.016

using namespace graphite2;

////    SHIFT-COLLIDER    ////

// Initialize the Collider to hold the basic movement limits for the
// target slot, the one we are focusing on fixing.
void ShiftCollider::initSlot(Segment *seg, Slot *aSlot, const Rect &limit, float margin,
    const Position &currShift, int dir, json * const dbgout)
{
    int i;
    float max, min;
    const GlyphCache &gc = seg->getFace()->glyphs();
    unsigned short gid = aSlot->gid();
//    if (currShift.x != 0. || currShift.y != 0.)
//        _limit = Rect(limit.bl - currShift, limit.tr - currShift);
//    else
        _limit = limit;
    // For a ShiftCollider, these indices indicate which vector we are moving by:
    for (i = 0; i < 4; ++i)
    {
        switch (i) {
            case 0 :	// x direction
                min = limit.bl.x + aSlot->origin().x + gc.getBoundingMetric(gid, 0);
                max = limit.tr.x + aSlot->origin().x + gc.getBoundingMetric(gid, 2);
                break;
            case 1 :	// y direction
                min = limit.bl.y + aSlot->origin().y + gc.getBoundingMetric(gid, 1);
                max = limit.tr.y + aSlot->origin().y + gc.getBoundingMetric(gid, 3);
                break;
            case 2 :	// sum (negatively sloped diagonal boundaries)
                min = 2.f * std::max(limit.bl.x, -limit.tr.y) + aSlot->origin().x + aSlot->origin().y
                    + gc.getBoundingMetric(gid, 4);
                max = 2.f * std::min(limit.tr.x, -limit.bl.y) + aSlot->origin().x + aSlot->origin().y
                    + gc.getBoundingMetric(gid, 6);
                break;
            case 3 :	// diff (positively sloped diagonal boundaries)
                min = 2.f * std::max(limit.bl.x, limit.bl.y) + aSlot->origin().x + aSlot->origin().y
                    + gc.getBoundingMetric(gid, 5);
                max = 2.f * std::min(limit.tr.x, limit.tr.y) + aSlot->origin().x + aSlot->origin().y
                    + gc.getBoundingMetric(gid, 7);
                break;
        }
        _ranges[i].clear();
        _ranges[i].add(min, max);

        // Debugging:
        _rawRanges[i].clear();
        _rawRanges[i].add(min, max);
        _removals[i].clear();
        _gidNear[i].clear();
        _subNear[i].clear();
        _subTarget[i].clear();
    }
    _target = aSlot;
    if ((dir & 1) == 0)
    {
        // For LTR, switch and negate x limits.
        _limit.bl.x = -1 * limit.tr.x;
        _limit.tr.x = -1 * limit.bl.x;
    }
    _margin = margin;
    _currShift = currShift;
}

// Adjust the movement limits for the target to avoid having it collide
// with the given slot. Also determine if there is in fact a collision
// between the target and the given slot.
bool ShiftCollider::mergeSlot(Segment *seg, Slot *slot, const Position &currShift, GR_MAYBE_UNUSED json * const dbgout )
{
    bool isCol = true;
    const float tx = _target->origin().x + _currShift.x;
    const float ty = _target->origin().y + _currShift.y;
    const float td = tx - ty;
    const float ts = tx + ty;
    const float sx = slot->origin().x + currShift.x;
    const float sy = slot->origin().y + currShift.y;
    const float sd = sx - sy;
    const float ss = sx + sy;
    float vmin, vmax;
    float omin, omax, otmin, otmax;
    float cmin, cmax;
    const GlyphCache &gc = seg->getFace()->glyphs();
    const unsigned short gid = slot->gid();
    const unsigned short tgid = _target->gid();
    
    // Process main bounding octabox.
    for (int i = 0; i < 4; ++i)
    {
        uint16 m = _margin / (i > 1 ? ISQRT2 : 1.);  // adjusted margin by depending on whether the vector is diagonal
        switch (i) {
            case 0 :	// x direction
                vmin = std::max(std::max(gc.getBoundingMetric(gid, 0) + sx,
                                gc.getBoundingMetric(gid, 5) + sd + gc.getBoundingMetric(tgid, 1) + ty),
                                gc.getBoundingMetric(gid, 4) + ss - gc.getBoundingMetric(tgid, 3) - ty);
                vmax = std::min(std::min(gc.getBoundingMetric(gid, 2) + sx,
                                gc.getBoundingMetric(gid, 7) + sd + gc.getBoundingMetric(tgid, 3) + ty),
                                gc.getBoundingMetric(gid, 6) + ss - gc.getBoundingMetric(tgid, 1) - ty);
                otmin = gc.getBoundingMetric(tgid, 1) + ty;
                otmax = gc.getBoundingMetric(tgid, 3) + ty;
                omin = gc.getBoundingMetric(gid, 1) + sy;
                omax = gc.getBoundingMetric(gid, 3) + sy;
                cmin = _limit.bl.x + tx + gc.getBoundingMetric(tgid, 0);
                cmax = _limit.tr.x + tx + gc.getBoundingMetric(tgid, 2);
                break;
            case 1 :	// y direction
                vmin = std::max(std::max(gc.getBoundingMetric(gid, 1) + sy,
                                gc.getBoundingMetric(tgid, 0) + tx - gc.getBoundingMetric(gid, 7) - sd),
                                gc.getBoundingMetric(gid, 4) + ss - gc.getBoundingMetric(tgid, 2) - tx);
                vmax = std::min(std::min(gc.getBoundingMetric(gid, 3) + sy,
                                gc.getBoundingMetric(tgid, 2) + tx - gc.getBoundingMetric(gid, 5) - sd),
                                gc.getBoundingMetric(gid, 6) + ss - gc.getBoundingMetric(tgid, 0) - tx);
                otmin = gc.getBoundingMetric(tgid, 0) + tx;
                otmax = gc.getBoundingMetric(tgid, 2) + tx;
                omin = gc.getBoundingMetric(gid, 0) + sx;
                omax = gc.getBoundingMetric(gid, 2) + sx;
                cmin = _limit.bl.y + ty + gc.getBoundingMetric(tgid, 1);
                cmax = _limit.tr.y + ty + gc.getBoundingMetric(tgid, 3);
                break;
            case 2 :    // sum - moving along the positively-sloped vector, so the boundaries are the
                        // negatively-sloped boundaries.
                vmin = std::max(std::max(gc.getBoundingMetric(gid, 4) + ss,
                                2 * gc.getBoundingMetric(gid, 1) + 2 * sy + gc.getBoundingMetric(tgid, 5) + td),
                                2 * gc.getBoundingMetric(gid, 0) + 2 * sx - gc.getBoundingMetric(tgid, 7) - td);
                vmax = std::min(std::min(gc.getBoundingMetric(gid, 6) + ss,
                                2 * gc.getBoundingMetric(gid, 3) + 2 * sy + gc.getBoundingMetric(tgid, 7) + td),
                                2 * gc.getBoundingMetric(gid, 2) + 2 * sx - gc.getBoundingMetric(tgid, 5) - td);
                otmin = gc.getBoundingMetric(tgid, 5) + td;
                otmax = gc.getBoundingMetric(tgid, 7) + td;
                omin = gc.getBoundingMetric(gid, 5) + sd;
                omax = gc.getBoundingMetric(gid, 7) + sd;
                cmin = _limit.bl.x + _limit.bl.y + ts + gc.getBoundingMetric(tgid, 4);
                cmax = _limit.tr.x + _limit.tr.y + ts + gc.getBoundingMetric(tgid, 6);
                break;
            case 3 :    // diff - moving along the negatively-sloped vector, so the boundaries are the
                        // positively-sloped boundaries.
                vmin = std::max(std::max(gc.getBoundingMetric(gid, 5) + sd,
                                2 * gc.getBoundingMetric(gid, 0) + 2 * sx - gc.getBoundingMetric(tgid, 6) - ts),
                                gc.getBoundingMetric(tgid, 4) + ts - 2 * gc.getBoundingMetric(gid, 3) - 2 * sy);
                vmax = std::min(std::min(gc.getBoundingMetric(gid, 7) + sd,
                                2 * gc.getBoundingMetric(gid, 2) + 2 * sx - gc.getBoundingMetric(tgid, 4) - ts),
                                gc.getBoundingMetric(tgid, 6) + ts - 2 * gc.getBoundingMetric(gid, 1) - 2 * sy);
                otmin = gc.getBoundingMetric(tgid, 4) + ts;
                otmax = gc.getBoundingMetric(tgid, 6) + ts;
                omin = gc.getBoundingMetric(gid, 4) + ss;
                omax = gc.getBoundingMetric(gid, 6) + ss;
                cmin = _limit.bl.x - _limit.tr.y + td + gc.getBoundingMetric(tgid, 5);
                cmax = _limit.tr.x - _limit.bl.y + td + gc.getBoundingMetric(tgid, 7);
                break;
            default :
                continue;
        }
        // if ((vmin < cmin - m && vmax < cmin - m) || (vmin > cmax + m && vmax > cmax + m)
        //    // or it is offset in the opposite dimension:
        //    || (omin < otmin - m && omax < otmin - m) || (omin > otmax + m && omax > otmax + m))
        if (vmax < cmin - m || vmin > cmax + m || omax < otmin - m || omin > otmax + m)
        {
            isCol = false;
            continue;
        }
        
        // Process sub-boxes that are defined for this glyph.
        // We only need to do this if there was in fact a collision with the main octabox.
        uint8 numsub = gc.numSubBounds(gid);
        if (numsub > 0)
        {
            bool anyhits = false;
            for (int j = 0; j < numsub; ++j)
            {
                switch (i) {
                    case 0 :    // x
                        vmin = std::max(std::max(gc.getSubBoundingMetric(gid, j, 0) + sx,
                                        gc.getSubBoundingMetric(gid, j, 5) + sd + gc.getBoundingMetric(tgid, 1) + ty),
                                        gc.getSubBoundingMetric(gid, j, 4) + ss - gc.getBoundingMetric(tgid, 3) - ty);
                        vmax = std::min(std::min(gc.getSubBoundingMetric(gid, j, 2) + sx,
                                        gc.getSubBoundingMetric(gid, j, 7) + sd + gc.getBoundingMetric(tgid, 3) + ty),
                                        gc.getSubBoundingMetric(gid, j, 6) + ss - gc.getBoundingMetric(tgid, 1) - ty);
                        omin = gc.getSubBoundingMetric(gid, j, 1) + sy;
                        omax = gc.getSubBoundingMetric(gid, j, 3) + sy;
                        break;
                    case 1 :    // y
                        vmin = std::max(std::max(gc.getSubBoundingMetric(gid, j, 1) + sy,
                                        gc.getBoundingMetric(tgid, 0) + tx - gc.getSubBoundingMetric(gid, j, 7) - sd),
                                        gc.getSubBoundingMetric(gid, j, 4) + ss - gc.getBoundingMetric(tgid, 2) - tx);
                        vmax = std::min(std::min(gc.getSubBoundingMetric(gid, j, 3) + sy,
                                        gc.getBoundingMetric(tgid, 2) + tx - gc.getSubBoundingMetric(gid, j, 5) - sd),
                                        gc.getSubBoundingMetric(gid, j, 6) + ss - gc.getBoundingMetric(tgid, 0) - tx);
                        omin = gc.getSubBoundingMetric(gid, j, 0) + sx;
                        omax = gc.getSubBoundingMetric(gid, j, 2) + sx;
                        break;
                    case 2 :    // sum
                        vmin = std::max(std::max(gc.getSubBoundingMetric(gid, j, 4) + ss,
                                        2 * gc.getSubBoundingMetric(gid, j, 1) + 2 * sy + gc.getBoundingMetric(tgid, 5) + td),
                                        2 * gc.getSubBoundingMetric(gid, j, 0) + 2 * sx - gc.getBoundingMetric(tgid, 7) - td);
                        vmax = std::min(std::min(gc.getSubBoundingMetric(gid, j, 6) + ss,
                                        2 * gc.getSubBoundingMetric(gid, j, 3) + 2 * sy + gc.getBoundingMetric(tgid, 7) + td),
                                        2 * gc.getSubBoundingMetric(gid, j, 2) + 2 * sx - gc.getBoundingMetric(tgid, 5) - td);
                        omin = gc.getSubBoundingMetric(gid, j, 5) + sd;
                        omax = gc.getSubBoundingMetric(gid, j, 7) + sd;
                        break;
                    case 3 :    // diff
                        vmin = std::max(std::max(gc.getSubBoundingMetric(gid, j, 5) + sd,
                                        2 * gc.getSubBoundingMetric(gid, j, 0) + 2 * sx - gc.getBoundingMetric(tgid, 6) - ts),
                                        gc.getBoundingMetric(tgid, 4) + ts - 2 * gc.getSubBoundingMetric(gid, j, 3) - 2 * sy);
                        vmax = std::min(std::min(gc.getSubBoundingMetric(gid, j, 7) + sd,
                                        2 * gc.getSubBoundingMetric(gid, j, 2) + 2 * sx - gc.getBoundingMetric(tgid, 4) - ts),
                                        gc.getBoundingMetric(tgid, 6) + ts - 2 * gc.getSubBoundingMetric(gid, j, 1) - 2 * sy);
                        omin = gc.getSubBoundingMetric(gid, j, 4) + ss;
                        omax = gc.getSubBoundingMetric(gid, j, 6) + ss;
                        break;
                }
                // if ((vmin < cmin - m && vmax < cmin - m) || (vmin > cmax + m && vmax > cmax + m)
                //     		|| (omin < otmin - m && omax < otmin - m) || (omin > otmax + m && omax > otmax + m))
                if (vmax < cmin - m || vmin > cmax + m || omax < otmin - m || omin > otmax + m)
                    continue;
                _ranges[i].remove(vmin, vmax);
                anyhits = true;
                
                IntervalSet::tpair dbg(vmin, vmax); // debugging
                _removals[i].append(dbg);           // debugging
                _gidNear[i].push_back(gid);       // debugging
                _subNear[i].push_back(j);         // debugging
            }
            if (!anyhits)
                isCol = false;
        }
        else
        {
            _ranges[i].remove(vmin, vmax);

            IntervalSet::tpair dbg(vmin, vmax); // debugging
            _removals[i].append(dbg);           // debugging
            _gidNear[i].push_back(gid);       // debugging
            _subNear[i].push_back(-1);        // debugging
        }
    }
    return isCol;
}


// Figure out where to move the target glyph to, and return the amount to shift by.
Position ShiftCollider::resolve(Segment *seg, bool &isCol, GR_MAYBE_UNUSED json * const dbgout)
{
    const GlyphCache &gc = seg->getFace()->glyphs();
    int gid = _target->gid();
    float margin;
    float tlen, tleft, tbase;
    float totald = std::numeric_limits<float>::max() / 2.;
    Position totalp = _currShift;
    // float cmax, cmin;
    bool isGoodFit, tIsGoodFit = false;
    IntervalSet aFit;
    int flags = seg->collisionInfo(_target)->flags();
#if !defined GRAPHITE2_NTRACING
    if (dbgout)
    {
        *dbgout << json::object // slot
                << "slot" << objectid(dslot(seg, _target)) 
                << "limit" << _limit
                << "target" << json::object
                    << "origin" << _target->origin()
                    << "currShift" << _currShift
                    << "bbox" << seg->theGlyphBBoxTemporary(_target->gid())
                    << "slantbox" << seg->getFace()->glyphs().slant(_target->gid())
                    << "fix" << "shift";
        *dbgout     << json::close // target object
        	      << "vectors" << json::array;
    }
#endif
    for (int i = 0; i < 4; ++i)
    {
        float bestc = std::numeric_limits<float>::max();
        float bestd = bestc;
        float cmin, cmax;
        // Calculate the margin depending on whether we are moving diagonally or not:
        margin = seg->collisionInfo(_target)->margin() * (i > 1 ? ISQRT2 : 1.f);
        switch (i) {
            case 0 :	// x direction
                tlen = gc.getBoundingMetric(gid, 2) - gc.getBoundingMetric(gid, 0);
                tleft = _target->origin().x + _currShift.x + gc.getBoundingMetric(gid, 0);
                tbase = tleft - _currShift.x;
                cmin = _limit.bl.x + tleft;
                cmax = _limit.tr.x + tleft;
                break;
            case 1 :	// y direction
                tlen = gc.getBoundingMetric(gid, 3) - gc.getBoundingMetric(gid, 1);
                tleft = _target->origin().y + _currShift.y + gc.getBoundingMetric(gid, 1);
                tbase = tleft - _currShift.y;
                cmin = _limit.bl.y + tleft;
                cmax = _limit.tr.y + tleft;
                break;
            case 2 :	// sum (negatively-sloped diagonals)
                tlen = gc.getBoundingMetric(gid, 6) - gc.getBoundingMetric(gid, 4);
                tleft = _target->origin().x + _target->origin().y + _currShift.x + _currShift.y + gc.getBoundingMetric(gid, 4);
                tbase = tleft - _currShift.x - _currShift.y;
                cmin = _limit.bl.x + _limit.bl.y + tleft;
                cmax = _limit.tr.x + _limit.tr.y + tleft;
                break;
            case 3 :	// diff (positively-sloped diagonals)
                tlen = gc.getBoundingMetric(gid, 7) - gc.getBoundingMetric(gid, 5);
                tleft = _target->origin().x - _target->origin().y + _currShift.x - _currShift.y + gc.getBoundingMetric(gid, 5);
                tbase = tleft - _currShift.x + _currShift.y;
                cmin = _limit.bl.x - _limit.tr.y + tleft;
                cmax = _limit.tr.x - _limit.bl.y + tleft;
                break;
        }
        isGoodFit = true;
        aFit = _ranges[i].locate(tbase, tbase + tlen);
        bestd = aFit.findBestWithMarginAndLimits(0., margin / (i > 1 ? ISQRT2 : 1.), cmin - tbase, cmax - tbase, isGoodFit);
        Position testp;
        switch (i) {
            case 0 : testp = Position(bestd, _currShift.y); break;
            case 1 : testp = Position(_currShift.x, bestd); break;
            case 2 : testp = Position(0.5 * (bestd + _currShift.x - _currShift.y), 0.5 * (bestd - _currShift.x + _currShift.y)); break;
            case 3 : testp = Position(0.5 * (bestd + _currShift.x + _currShift.y), 0.5 * (_currShift.x + _currShift.y - bestd)); break;
        }
#if !defined GRAPHITE2_NTRACING
        if (dbgout)
        {
            *dbgout << json::object // vector
                    << "testleft" << tleft
                    << "testlen" << tlen;
            
            *dbgout << "rawRanges" << json::flat << json::array;
            for (IntervalSet::ivtpair s = _rawRanges[i].begin(), e = _rawRanges[i].end(); s != e; ++s)
                *dbgout << Position(s->first, s->second);
            *dbgout << json::close // rawRanges array
                << "removals" << json::array;  						
            int gi = 0;
            for (IntervalSet::ivtpair s = _removals[i].begin(), e = _removals[i].end(); s != e; ++s, ++gi)
                *dbgout << json::flat << json::array << _gidNear[i][gi] << _subNear[i][gi] << Position(s->first, s->second) << json::close;
            *dbgout << json::close; // removals array
            	
            *dbgout << "ranges";
            debug(dbgout, seg, i);

            *dbgout << "fits" << json::flat << json::array;
            for (IntervalSet::ivtpair s = aFit.begin(), e = aFit.end(); s != e; ++s)
                *dbgout << Position(s->first, s->second);
            *dbgout << json::close // fits array
                    << "bestfit" << bestd
                << json::close; // vectors object
        }
#endif
        bestd = testp.x * testp.x + testp.y * testp.y;
        //bestd = _ranges[i].bestfit(tleft - margin, tleft + tlen + margin, isGoodFit);
        // bestd += bestd > 0.f ? -margin : margin;
        
        // See if this direction is the best one so far to move in.
        // Don't replace a good-fit move with a bad-fit move.
        if ((isGoodFit && !tIsGoodFit) || ((isGoodFit || !tIsGoodFit) && fabs(bestd) < totald))
        {
            totald = fabs(bestd);
            tIsGoodFit = isGoodFit;
            totalp = testp;
        }
    }  // end of loop over 4 directions
#if !defined GRAPHITE2_NTRACING
    if (dbgout)
    {
        *dbgout << json::close // vectors array
            << "result" << totalp << json::close; // slot object
    }
#endif
    isCol = !tIsGoodFit;
    return totalp;
}

////    KERN-COLLIDER    ////

static float get_left(Segment *seg, const Slot *s, float y)
{
    const GlyphCache &gc = seg->getFace()->glyphs();
    unsigned short gid = s->gid();
    float sx = s->origin().x;
    float sy = s->origin().y;
    uint8 numsub = gc.numSubBounds(gid);
    float res = 1e38;

    if (numsub > 0)
    {
        for (int i = 0; i < numsub; ++i)
        {
            if (sy + gc.getSubBoundingMetric(gid, i, 1) > y && sy + gc.getSubBoundingMetric(gid, i, 3) < y)
                continue;
            float x = sx + gc.getSubBoundingMetric(gid, i, 0);
            if (x < res)
            {
                x = std::max(sx - sy + gc.getSubBoundingMetric(gid, i, 7) + y,
                            std::max(sx + sy + gc.getSubBoundingMetric(gid, i, 4) - y, x));
                if (x < res)
                    res = x;
            }
        }
    }
    else
        res = std::max(sx + gc.getBoundingMetric(gid, 0),
                    std::max(sx - sy + gc.getBoundingMetric(gid, 7) + y,
                             sx + sy + gc.getBoundingMetric(gid, 4) -y));
    return res;
}

static float get_right(Segment *seg, const Slot *s, float y)
{
    const GlyphCache &gc = seg->getFace()->glyphs();
    unsigned short gid = s->gid();
    float sx = s->origin().x;
    float sy = s->origin().y;
    uint8 numsub = gc.numSubBounds(gid);
    float res = -1e38;

    if (numsub > 0)
    {
        for (int i = 0; i < numsub; ++i)
        {
            if (sy + gc.getSubBoundingMetric(gid, i, 1) > y && sy + gc.getSubBoundingMetric(gid, i, 3) < y)
                continue;
            float x = sx + gc.getSubBoundingMetric(gid, i, 2);
            if (x > res)
            {
                x = std::min(sx - sy + gc.getSubBoundingMetric(gid, i, 5) + y,
                            std::min(sx + sy + gc.getSubBoundingMetric(gid, i, 6) - y, x));
                if (x > res)
                    res = x;
            }
        }
    }
    else
        res = std::min(sx + gc.getBoundingMetric(gid, 2),
                    std::min(sx - sy + gc.getBoundingMetric(gid, 5) + y,
                             sx + sy + gc.getBoundingMetric(gid, 6) -y));
    return res;
}

     

void KernCollider::initSlot(Segment *seg, Slot *aSlot, const Rect &limit, float margin, const Position &currShift,
    float currKern, int dir, json * const dbgout)
{
    const GlyphCache &gc = seg->getFace()->glyphs();
    unsigned short gid = aSlot->gid();
    const Slot *base = aSlot;
    const Slot *last = aSlot;
    const Slot *s;
    int maxid = aSlot->index();
    float sliceWidth;
    while (base->attachedTo()) base = base->attachedTo();

    _maxy = -1e38;
    _miny = 1e38;
    _xbound = (dir & 1) ? 1e38 : -1e38;
    for (s = base; s; s = s->nextInCluster(s))
    {
        if (s->index() > maxid)
        {
            last = s;
            maxid = s->index();
        }
        _maxy = std::max(_maxy, s->origin().y + gc.getBoundingMetric(s->gid(), 3));
        _miny = std::min(_miny, s->origin().y + gc.getBoundingMetric(s->gid(), 1));
    }
    _numSlices = int((_maxy - _miny) / margin + 1.);
    sliceWidth = (_maxy - _miny) / _numSlices;
    _edges.clear();
    _edges.insert(_edges.begin(), _numSlices, dir & 1 ? 1e38 : -1e38);

    for (s = base; s; s = s->nextInCluster(s))
    {
        float x = s->origin().x + gc.getBoundingMetric(s->gid(), (dir & 1 ? 0 : 2));
        int smin = std::max(0, int((s->origin().y + gc.getBoundingMetric(s->gid(), 1) - _miny) / (_maxy - _miny) * _numSlices));
        int smax = std::min(_numSlices - 1, int((s->origin().y + gc.getBoundingMetric(s->gid(), 3) - _miny) / (_maxy - _miny) * _numSlices + 1));
        for (int i = smin; i < smax; ++i)
        {
            float t;
            float y = _miny + (smin + .5) * sliceWidth;
            if ((dir & 1) && x < _edges[i])
            {
                t = get_left(seg, s, y);
                if (t < _edges[i])
                {
                    _edges[i] = t;
                    if (t < _xbound)
                        _xbound = t;
                }
            }
            else if (!(dir & 1) && x > _edges[i])
            {
                t = get_right(seg, s, y);
                if (t > _edges[i])
                {
                    _edges[i] = t;
                    if (t > _xbound)
                        _xbound = t;
                }
            }
        }
    }
    _mingap = 1e38;
    _target = aSlot;
    _margin = margin;
    _currShift = currShift;
}


bool KernCollider::mergeSlot(Segment *seg, Slot *slot, const Position &currShift, int dir, json * const dbgout)
{
    const GlyphCache &gc = seg->getFace()->glyphs();
    uint16 gid = slot->gid();
    const float sx = slot->origin().x + currShift.x;
    const float sy = slot->origin().y + currShift.y;
    bool res = false;
    int smin = std::max(0, int((sy + gc.getBoundingMetric(gid, 1) - _miny) / (_maxy - _miny) * _numSlices));
    int smax = std::min(_numSlices - 1, int((sy + gc.getBoundingMetric(gid, 3) - _miny) / (_maxy - _miny) * _numSlices + 1));
    float sliceWidth = (_maxy - _miny) / _numSlices;

    if (dir & 1)
    {
        float x = sx + gc.getBoundingMetric(gid, 0);
        if (x < _xbound)
            return false;
        for (int i = smin; i < smax; ++i)
        {
            float t;
            float y = _miny + (smin + .5) * sliceWidth;
            if (x < _edges[i])
            {
                t = _edges[i] - get_left(seg, slot, y);
                if (t < _mingap)
                    _mingap = t;
            }
        }
    }
    else
    {
        float x = sx + gc.getBoundingMetric(gid, 2);
        if (x > _xbound)
            return false;
        for (int i = smin; i < smax; ++i)
        {
            float t;
            float y = _miny + (smin + .5) * sliceWidth;
            if (x > _edges[i])
            {
                t = get_right(seg, slot, y) - _edges[i];
                if (t < _mingap)
                    _mingap = t;
            }
        }
    }
    return false;   // what can we say?
}

// Return the amount to kern by.
Position KernCollider::resolve(Segment *seg, int dir, GR_MAYBE_UNUSED json * const dbgout)
{
    return Position((1 - 2*(dir & 1)) * _mingap, 0.);
}

// Initialize the structure for the given slot.
SlotCollision::SlotCollision(Segment *seg, Slot *slot)
{
    uint16 gid = slot->gid();
    uint16 aCol = seg->silf()->aCollision();
/*    _limit = Rect(Position(seg->glyphAttr(gid, aCol+1) + slot->origin().x,
                           seg->glyphAttr(gid, aCol+2) + slot->origin().y),
                  Position(seg->glyphAttr(gid, aCol+3) + slot->origin().x,
                           seg->glyphAttr(gid, aCol+4) + slot->origin().y)); */
    _limit = Rect(Position(seg->glyphAttr(gid, aCol+1), seg->glyphAttr(gid, aCol+2)),
                  Position(seg->glyphAttr(gid, aCol+3), seg->glyphAttr(gid, aCol+4)));
    _margin = seg->glyphAttr(gid, aCol+5);
    _flags = seg->glyphAttr(gid, aCol);
}

float SlotCollision::getKern(int dir) const
{
    if ((_flags & SlotCollision::COLL_KERN) != 0)
        return float(_shift.x * ((dir & 1) ? -1 : 1));
    else
    	return 0;
}


