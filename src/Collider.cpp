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

using namespace graphite2;

////    SHIFT-COLLIDER    ////

// Initialize the Collider to hold the basic movement limits for the
// target slot, the one we are focusing on fixing.
void ShiftCollider::initSlot(Segment *seg, Slot *aSlot, const Rect &limit, float margin,
    const Position &currshift, float currKern, int dir, json * const dbgout)
{
    int i;
    float max, min;
    const GlyphCache &gc = seg->getFace()->glyphs();
    unsigned short gid = aSlot->gid();
    // For a ShiftCollider, these indices indicate which vector we are moving by:
    for (i = 0; i < 4; ++i)
    {
        switch (i) {
            case 0 :	// x direction
                min = limit.bl.x + aSlot->origin().x + currKern + gc.getBoundingMetric(gid, 0);
                max = limit.tr.x + aSlot->origin().x + currKern + gc.getBoundingMetric(gid, 2);
                break;
            case 1 :	// y direction
                min = limit.bl.y + aSlot->origin().y + gc.getBoundingMetric(gid, 1);
                max = limit.tr.y + aSlot->origin().y + gc.getBoundingMetric(gid, 3);
                break;
            case 2 :	// sum (negatively sloped diagonal)
                min = 2.f * std::max(limit.bl.x, -limit.tr.y) + aSlot->origin().x + currKern + aSlot->origin().y
                    + gc.getBoundingMetric(gid, 4);
                max = 2.f * std::min(limit.tr.x, -limit.bl.y) + aSlot->origin().x + currKern + aSlot->origin().y
                    + gc.getBoundingMetric(gid, 6);
                break;
            case 3 :	// diff (positively sloped diagonal)
                min = 2.f * std::max(limit.bl.x, limit.bl.y) + aSlot->origin().x + currKern - aSlot->origin().y
                    + gc.getBoundingMetric(gid, 5);
                max = 2.f * std::min(limit.tr.x, limit.tr.y) + aSlot->origin().x + currKern - aSlot->origin().y
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
    }
    _target = aSlot;
    _limit = limit;
    if ((dir & 1) == 0)
    {
        // For LTR, switch and negate x limits.
        _limit.bl.x = -1 * limit.tr.x;
        _limit.tr.x = -1 * limit.bl.x;
    }
    _margin = margin;
    _currshift = currshift;
    _kern = currKern; // should be integrated into _currshift
}

// Adjust the movement limits for the target to avoid having it collide
// with the given slot. Also determine if there is in fact a collision
// between the target and the given slot.
bool ShiftCollider::mergeSlot(Segment *seg, Slot *slot, const Position &currshift, const float currKern,
    GR_MAYBE_UNUSED bool ignoreForKern, GR_MAYBE_UNUSED json * const dbgout )
{

//if (ignoreForKern)
//	return false;
	
    bool isCol = true;
    const float tx = _target->origin().x + _kern + _currshift.x;
    const float ty = _target->origin().y         + _currshift.y;
    const float td = tx - ty;
    const float ts = tx + ty;
    const float sx = slot->origin().x + currKern + currshift.x;
    const float sy = slot->origin().y            + currshift.y;
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
        uint16 m = _margin * (i > 1 ? ISQRT2 : 1.);  // adjusted margin by depending on whether the vector is diagonal
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
            case 2 :    // sum
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
            case 3 :    // diff
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
    float tlen, tleft;
    float totald = std::numeric_limits<float>::max();
    Position totalp;
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
                    << "kern" << _kern
                    << "currshift" << _currshift
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
                tleft = _target->origin().x + _kern + _currshift.x + gc.getBoundingMetric(gid, 0);
                cmin = _limit.bl.x + tleft;
                cmax = _limit.tr.x + tleft;
                break;
            case 1 :	// y direction
                tlen = gc.getBoundingMetric(gid, 3) - gc.getBoundingMetric(gid, 1);
                tleft = _target->origin().y + _currshift.y + gc.getBoundingMetric(gid, 1);
                cmin = _limit.bl.y + tleft;
                cmax = _limit.tr.y + tleft;
                break;
            case 2 :	// sum (negatively-sloped diagonals)
                tlen = gc.getBoundingMetric(gid, 6) - gc.getBoundingMetric(gid, 4);
                tleft = _target->origin().x + _kern + _target->origin().y + _currshift.x + _currshift.y + gc.getBoundingMetric(gid, 4);
                cmin = _limit.bl.x + _limit.bl.y + tleft;
                cmax = _limit.tr.x + _limit.tr.y + tleft;
                break;
            case 3 :	// diff (positively-sloped diagonals)
                tlen = gc.getBoundingMetric(gid, 7) - gc.getBoundingMetric(gid, 5);
                tleft = _target->origin().x + _kern - _target->origin().y + _currshift.x - _currshift.y + gc.getBoundingMetric(gid, 5);
                cmin = _limit.bl.x - _limit.tr.y + tleft;
                cmax = _limit.tr.x - _limit.bl.y + tleft;
                break;
        }
        isGoodFit = true;
        aFit = _ranges[i].locate(tleft, tleft + tlen);
        bestd = aFit.findBestWithMarginAndLimits(0., margin / (i > 1 ? ISQRT2 : 1.), cmin - tleft, cmax - tleft, isGoodFit);
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
        //bestd = _ranges[i].bestfit(tleft - margin, tleft + tlen + margin, isGoodFit);
        // bestd += bestd > 0.f ? -margin : margin;
        
        // See if this direction is the best one so far to move in.
        // Don't replace a good-fit move with a bad-fit move.
        if ((isGoodFit && !tIsGoodFit) || ((isGoodFit || !tIsGoodFit) && fabs(bestd) < totald))
        {
            totald = fabs(bestd);
            tIsGoodFit = isGoodFit;
            switch (i) {
                case 0 : totalp = Position(bestd, 0.); break;
                case 1 : totalp = Position(0., bestd); break;
                case 2 : totalp = Position(bestd * ISQRT2, bestd * ISQRT2); break;
                case 3 : totalp = Position(bestd * ISQRT2, -bestd * ISQRT2); break;
            }
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

void KernCollider::initSlot(Segment *seg, Slot *aSlot, const Rect &limit, float margin, const Position &currshift,
    float currKern, int dir, json * const dbgout)
{
    const GlyphCache &gc = seg->getFace()->glyphs();
    unsigned short gid = aSlot->gid();
    float minx = limit.bl.x + aSlot->origin().x + currKern;
    float maxx = limit.tr.x + aSlot->origin().x + currKern;
    _miny = aSlot->origin().y + gc.getBoundingMetric(gid, 1);
    _maxy = aSlot->origin().y + gc.getBoundingMetric(gid, 3);
    int i;
    // For a KernCollider, these indices indicate which row of the subbox grid the collision is on (0=bottom, 3=top):
    int numtsub = gc.numSubBounds(gid);
    if (numtsub == 0)
    {
        for (i = 0; i < 4; ++i)
        {
            _ranges[i].clear();
            _ranges[i].add(minx + gc.getBoundingMetric(gid, 0), maxx + gc.getBoundingMetric(gid, 2));
            // Debugging:
            _rawRanges[i].clear();
            _rawRanges[i].add(minx + gc.getBoundingMetric(gid, 0), maxx + gc.getBoundingMetric(gid, 2));
            _removals[i].clear();
            _gidNear[i].clear();
            _subNear[i].clear();
        }
    }
    else
    {
        for (i = 0; i < 4; ++i)
        {
            _ranges[i].clear();
            // Debugging:
            _rawRanges[i].clear();
            _removals[i].clear();
            _gidNear[i].clear();
            _subNear[i].clear();
        }
        const float step = (_maxy - _miny) / 4; // height of subboxes
        const float bbBottom = _miny - aSlot->origin().y;
//#if !defined GRAPHITE2_NTRACING
//*dbgout << json::flat << json::object <<"bbTop" << gc.getBoundingMetric(gid, 3) << "bbBottom" << bbBottom 
//    << json::close;
//#endif
        for (i = 0; i < numtsub; ++i) // loop over subboxes
        {
        	// which row the sub-box is on (0= bottom, 3 = top)
            // 0.016 = 1/64 = (1/256)*4 which corresponds to possible rounding error (_maxy - _miny)/256.
            int row = int((gc.getSubBoundingMetric(gid, i, 1) - bbBottom) / step + 0.016);
            float ix = minx + gc.getSubBoundingMetric(gid, i, 0);  // left
            float ax = maxx + gc.getSubBoundingMetric(gid, i, 2);  // right
            _ranges[row].add(ix, ax); // TODO: don't just add - extend endpoints so we have a single range
            _rawRanges[row].add(ix, ax);
#if !defined GRAPHITE2_NTRACING
            *dbgout << json::object << "subbox" << i << "row" << row
                << "left" << gc.getSubBoundingMetric(gid, i, 0) << "right" << gc.getSubBoundingMetric(gid, i, 2)
                << "top" << gc.getSubBoundingMetric(gid, i, 3) << "bottom" << gc.getSubBoundingMetric(gid, i, 1) 
                << "minx" << minx << "maxx" << maxx 
                << json::close;
#endif
        }
    }
    _target = aSlot;
    _limit = limit;
    _margin = margin;
    _currshift = currshift;
    _kern = currKern;
}

// Adjust the x-dimension limits to avoid neighboring glyphs.
// it = subbox index of target, or -1; ig = subbox index of neighbor
bool KernCollider::removeXCovering(uint16 gid, uint16 tgid, const GlyphCache &gc, 
        float sx, float sy, float tx, float ty, int it, int ig, IntervalSet &range,
        int row)
{
    float vmin, vmax, omin, omax;
    float otmin, otmax, cmin, cmax; 
    float ss = sx + sy;
    float sd = sx - sy;
    if (ig < 0 && it < 0)
    {
        vmin = std::max(std::max(gc.getBoundingMetric(gid, 0) + sx,
                                 gc.getBoundingMetric(gid, 5) + sd + gc.getBoundingMetric(tgid, 1) + ty),
                        gc.getBoundingMetric(gid, 4) + ss - gc.getBoundingMetric(tgid, 3) - ty);
        vmax = std::min(std::min(gc.getBoundingMetric(gid, 2) + sx,
                                 gc.getBoundingMetric(gid, 7) + sd + gc.getBoundingMetric(tgid, 3) + ty),
                        gc.getBoundingMetric(gid, 6) + ss - gc.getBoundingMetric(tgid, 1) - ty);
        omin = gc.getBoundingMetric(gid, 1) + sy;
        omax = gc.getBoundingMetric(gid, 3) + sy;
        otmin = gc.getBoundingMetric(tgid, 1) + ty;
        otmax = gc.getBoundingMetric(tgid, 3) + ty;
        cmin = _limit.bl.x + tx + gc.getBoundingMetric(tgid, 0);
        cmax = _limit.tr.x + tx + gc.getBoundingMetric(tgid, 2);
    }
    else if (it < 0)
    {
        vmin = std::max(std::max(gc.getSubBoundingMetric(gid, ig, 0) + sx,
                        gc.getSubBoundingMetric(gid, ig, 5) + sd + gc.getBoundingMetric(tgid, 1) + ty),
                        gc.getSubBoundingMetric(gid, ig, 4) + ss - gc.getBoundingMetric(tgid, 3) - ty);
        vmax = std::min(std::min(gc.getSubBoundingMetric(gid, ig, 2) + sx,
                        gc.getSubBoundingMetric(gid, ig, 7) + sd + gc.getBoundingMetric(tgid, 3) + ty),
                        gc.getSubBoundingMetric(gid, ig, 6) + ss - gc.getBoundingMetric(tgid, 1) - ty);
        omin = gc.getSubBoundingMetric(gid, ig, 1) + sy;
        omax = gc.getSubBoundingMetric(gid, ig, 3) + sy;
        otmin = gc.getBoundingMetric(tgid, 1) + ty;
        otmax = gc.getBoundingMetric(tgid, 3) + ty;
        cmin = _limit.bl.x + tx + gc.getBoundingMetric(tgid, 0);
        cmax = _limit.tr.x + tx + gc.getBoundingMetric(tgid, 2);
    }
    else if (ig < 0)
    {
        vmin = std::max(std::max(gc.getBoundingMetric(gid, 0) + sx,
                                 gc.getBoundingMetric(gid, 5) + sd + gc.getSubBoundingMetric(tgid, it, 1) + ty),
                        gc.getBoundingMetric(gid, 4) + ss - gc.getSubBoundingMetric(tgid, it, 3) - ty);
        vmax = std::min(std::min(gc.getBoundingMetric(gid, 2) + sx,
                                 gc.getBoundingMetric(gid, 7) + sd + gc.getSubBoundingMetric(tgid, it, 3) + ty),
                        gc.getBoundingMetric(gid, 6) + ss - gc.getSubBoundingMetric(tgid, it, 1) - ty);
        omin = gc.getBoundingMetric(gid, 1) + sy;
        omax = gc.getBoundingMetric(gid, 3) + sy;
        otmin = gc.getSubBoundingMetric(tgid, it, 1) + ty;
        otmax = gc.getSubBoundingMetric(tgid, it, 3) + ty;
        cmin = _limit.bl.x + tx + gc.getSubBoundingMetric(tgid, it, 0);
        cmax = _limit.tr.x + tx + gc.getSubBoundingMetric(tgid, it, 2);
    }
    else
    {
        vmin = std::max(std::max(gc.getSubBoundingMetric(gid, ig, 0) + sx,
                        gc.getSubBoundingMetric(gid, ig, 5) + sd + gc.getSubBoundingMetric(tgid, it, 1) + ty),
                        gc.getSubBoundingMetric(gid, ig, 4) + ss - gc.getSubBoundingMetric(tgid, it, 3) - ty);
        vmax = std::min(std::min(gc.getSubBoundingMetric(gid, ig, 2) + sx,
                        gc.getSubBoundingMetric(gid, ig, 7) + sd + gc.getSubBoundingMetric(tgid, it, 3) + ty),
                        gc.getSubBoundingMetric(gid, ig, 6) + ss - gc.getSubBoundingMetric(tgid, it, 1) - ty);
        omin = gc.getSubBoundingMetric(gid, ig, 1) + sy;
        omax = gc.getSubBoundingMetric(gid, ig, 3) + sy;
        otmin = gc.getSubBoundingMetric(tgid, it, 1) + ty;
        otmax = gc.getSubBoundingMetric(tgid, it, 3) + ty;
        cmin = _limit.bl.x + tx + gc.getSubBoundingMetric(tgid, it, 0);
        cmax = _limit.tr.x + tx + gc.getSubBoundingMetric(tgid, it, 2);
    }

    if (vmax < cmin - _margin || vmin > cmax + _margin || omax < otmin - _margin || omin > otmax + _margin)
        return false;
    range.remove(vmin, vmax);
    
    IntervalSet::tpair dbg(vmin, vmax); // debugging
    _removals[0].append(dbg);         // debugging - [0] means horizontal vector
    _gidNear[0].push_back(gid);       // debugging
    _subNear[0].push_back(ig);        // debugging

    return true;
}

bool KernCollider::mergeSlot(Segment *seg, Slot *slot, const Position &currshift, const float currKern, bool ignoreForKern,
    json * const dbgout)
{
    if (ignoreForKern)
        return false;
        
    const GlyphCache &gc = seg->getFace()->glyphs();
    uint16 gid = slot->gid();
    uint16 tgid = _target->gid();
    const float sx = slot->origin().x + currKern + currshift.x;
    const float sy = slot->origin().y + currshift.y;
    const float smin = sy + gc.getBoundingMetric(gid, 1);
    const float smax = sy + gc.getBoundingMetric(gid, 3);
    const float step = (_maxy - _miny) / 4; // height of subboxes
    const float tx = _target->origin().x + _kern + _currshift.x;
    const float ty = _target->origin().y + _currshift.y; // _currshift.y should always be 0
    int numtsub = gc.numSubBounds(tgid);
    int numsub = gc.numSubBounds(gid);
    int ti, gi;
    bool res = false;
    
    if (smin > _maxy || smax < _miny) return false;
        
    if (numtsub > 0)
    {
        for (ti = 0; ti < numtsub; ++ti) // loop over subboxes
        {
            if (numsub > 0)
                for (gi = 0; gi < numsub; ++gi)  // this branch is O(N^2) - so don't use subboxes unless you really need them
                {
                    // Determine which level (row of the target's subbox grid) corresponds most closely
                    // to the neighboring glyph's subbox.
                    int level = int((gc.getSubBoundingMetric(gid, gi, 1) + sy - _miny - _currshift.y) / step);
                    if (level >=0 && level < 4)
                        res |= removeXCovering(gid, tgid, gc, sx, sy, tx, ty, ti, gi, _ranges[level], level);
                    // otherwise neighboring subbox is offset vertically enough not to worry about it.
                }
            else
            {
                // Determine which level (row of the target's subbox grid) corresponds most closely
                // to the neighboring glyph.
                int level = int((smin - _miny - _currshift.y) / step);
                if (level >= 0 && level < 4)
                    res |= removeXCovering(gid, tgid, gc, sx, sy, tx, ty, ti, -1, _ranges[level], level);
                // otherwise neighboring glyph is offset vertically enough not to worry about it.
            }
        }
    }
    else
    {
        if (numsub > 0)
            for (gi = 0; gi < numsub; ++gi)
                res |= removeXCovering(gid, tgid, gc, sx, sy, tx, ty, -1, gi, _ranges[0], 0);
        else
            res |= removeXCovering(gid, tgid, gc, sx, sy, tx, ty, -1, -1, _ranges[0], 0);
    }
//#if !defined GRAPHITE2_NTRACING
//    *dbgout << json::object 
//        << "merge-slot" << objectid(dslot(seg, slot))
//        << "origin" << slot->origin()
//        << "kern" << currKern
//        << "bbox" << seg->theGlyphBBoxTemporary(slot->gid())
//        << "collides" << res
//        << json::close;
//#endif

    return res;
}

// Return the amount to kern by.
Position KernCollider::resolve(Segment *seg, bool &isCol, GR_MAYBE_UNUSED json * const dbgout)
{
    const GlyphCache &gc = seg->getFace()->glyphs();
    uint16 tgid = _target->gid();
    int numtsub = gc.numSubBounds(tgid);
    float tx = _target->origin().x + _kern + _currshift.x;
    float ty = _target->origin().y + _currshift.y;
    IntervalSet targetRanges[4];
    IntervalSet aFit;
    float step = (_maxy - _miny) / 4;
    int i;
    bool isGood = false;
    float shiftx;

#if !defined GRAPHITE2_NTRACING
    if (dbgout)
    {
        *dbgout << json::object // slot
                << "slot" << objectid(dslot(seg, _target)) 
                << "limit" << _limit
                << "target" << json::object
                    << "origin" << _target->origin()
                    << "kern" << _kern
                    << "currshift" << _currshift
                    << "bbox" << seg->theGlyphBBoxTemporary(_target->gid())
                    << "slantbox" << seg->getFace()->glyphs().slant(_target->gid())
                    << "fix" << "kern"
                    << "complexFit" << ((gc.numSubBounds(tgid) > 0) ? "true" : "false");

        *dbgout << "vectors" << json::array; // vector
////                    << "testleft" << tleft
////                    << "testlen" << tlen;
        
        for (int row = 0; row < (numtsub ? 4 :1); row++)
        {
            *dbgout << json::object // vector
	                      << "rawRanges" << json::flat << json::array;
	          for (IntervalSet::ivtpair s = _rawRanges[row].begin(), e = _rawRanges[row].end(); s != e; ++s)
                *dbgout << Position(s->first, s->second);
	          *dbgout << json::close // rawRanges array
	              << "removals" << json::array;  						
	          int gi = 0;
	          for (IntervalSet::ivtpair s = _removals[row].begin(), e = _removals[row].end(); s != e; ++s, ++gi)
	              *dbgout << json::flat << json::array << _gidNear[row][gi] << _subNear[row][gi] << Position(s->first, s->second) << json::close;
                *dbgout << json::close; // removals array
	        	
	          *dbgout << "ranges";
	          debug(dbgout, seg, row);
	          *dbgout << json::close;  // vector object
        }
        *dbgout << json::close;  // vector arrays
        	
        *dbgout << json::close; // target object
    }
#endif
    if (numtsub > 0)
    {
        for (i = 0; i < numtsub; ++i)
        {
            float yi = ty + gc.getSubBoundingMetric(tgid, i, 1);
            if (_miny <= yi && yi < _maxy )
            {
                // 0.016 = 1/64 = (1/256)*4 which corresponds to possible rounding error (_maxy - _miny)/256.
                int level = int((yi - _miny) / step + 0.016);
                targetRanges[level].add(gc.getSubBoundingMetric(tgid, i, 0) + tx,
                            gc.getSubBoundingMetric(tgid, i, 2) + tx);
            }
        }
    }
    else
    {
        targetRanges[0].add(gc.getBoundingMetric(tgid, 0) + tx, gc.getBoundingMetric(tgid, 2) + tx);
    }

    for (i = 0; i < (numtsub ? 4 : 1); ++i)
    {
        IntervalSet subrowFit = _ranges[i].locate(targetRanges[i]);
        if (i == 0)
            aFit = subrowFit;
        else
            aFit.intersect(subrowFit);
    }
    
#if !defined GRAPHITE2_NTRACING
	*dbgout << "targetRanges" << json::flat << json::array;
	for (i = 0; i < 4; ++i) {
		for (IntervalSet::ivtpair s = targetRanges[i].begin(), e = targetRanges[i].end(); s != e; ++s) {
            *dbgout << i << Position(s->first, s->second);
        }
    }
    *dbgout << json::close; // targetRanges array

    *dbgout << "fits" << json::flat << json::array;
    for (IntervalSet::ivtpair s = aFit.begin(), e = aFit.end(); s != e; ++s)
                *dbgout << Position(s->first, s->second);
    *dbgout << json::close; // fits array
#endif

    shiftx = aFit.findBestWithMarginAndLimits(0., _margin, _limit.bl.x, _limit.tr.x, isGood);
    if (shiftx > -1e38)
        isCol = false;
    else
    {
        shiftx = 0.,
        isCol = true;
    }
    
#if !defined GRAPHITE2_NTRACING
    if (dbgout)
    {
        *dbgout << "result" << shiftx
            << json::close;
    }
#endif
   
    return Position(shiftx, 0);
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

const float SlotCollision::getKern(int dir) const
{
    if ((_flags & SlotCollision::COLL_KERN) != 0)
        return float(_shift.x * ((dir & 1) ? -1 : 1));
    else
    	  return 0;
}


