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
#include <string>
#include <functional>
#include "inc/Collider.h"
#include "inc/Segment.h"
#include "inc/Slot.h"
#include "inc/GlyphCache.h"

#define ISQRT2 0.707106781f

// Possible rounding error for subbox boundaries: 0.016 = 1/64 = 1/256 * 4 
// (values in font range from 0..256)
#define SUBBOX_RND_ERR 0.016

using namespace graphite2;

////    SHIFT-COLLIDER    ////

// Initialize the Collider to hold the basic movement limits for the
// target slot, the one we are focusing on fixing.
void ShiftCollider::initSlot(Segment *seg, Slot *aSlot, const Rect &limit, float margin, float marginWeight,
    const Position &currShift, const Position &currOffset, int dir, GR_MAYBE_UNUSED json * const dbgout)
{
    int i;
    float max, min;
    float shift, oshift;
    const GlyphCache &gc = seg->getFace()->glyphs();
    unsigned short gid = aSlot->gid();
    const BBox &bb = gc.getBoundingBBox(gid);
    const SlantBox &sb = gc.getBoundingSlantBox(gid);
    //float sx = aSlot->origin().x + currShift.x;
    //float sy = aSlot->origin().y + currShift.y;
    if (currOffset.x != 0. || currOffset.y != 0.)
        _limit = Rect(limit.bl - currOffset, limit.tr - currOffset);
    else
        _limit = limit;
    // For a ShiftCollider, these indices indicate which vector we are moving by:
    // each _ranges represents absolute space with respect to the origin of the slot. Thus take into account true origins but subtract the vmin for the slot
    for (i = 0; i < 4; ++i)
    {
        switch (i) {
            case 0 :	// x direction
                min = _limit.bl.x + aSlot->origin().x;
                max = _limit.tr.x + aSlot->origin().x;
                _len[i] = bb.xa - bb.xi;
                shift = 0; // currOffset.x;
                oshift = currOffset.y + currShift.y;
                _ranges[i].initialise<XY>(min, max - min, margin, marginWeight, shift, oshift, oshift);
                break;
            case 1 :	// y direction
                min = _limit.bl.y + aSlot->origin().y;
                max = _limit.tr.y + aSlot->origin().y;
                _len[i] = bb.ya - bb.yi;
                shift = 0; //currOffset.y;
                oshift = currOffset.x + currShift.x;
                _ranges[i].initialise<XY>(min, max - min, margin, marginWeight, shift, oshift, oshift);
                break;
            case 2 :	// sum (negatively sloped diagonal boundaries)
                min = -2 * std::min(currShift.x - _limit.bl.x, currShift.y - _limit.bl.y) + aSlot->origin().x + aSlot->origin().y + currShift.x + currShift.y;
                max = 2 * std::min(_limit.tr.x - currShift.x, _limit.tr.y - currShift.y) + aSlot->origin().x + aSlot->origin().y + currShift.x + currShift.y;
                _len[i] = sb.sa - sb.si;
                shift = 0; //currOffset.x + currOffset.y;
                oshift = currOffset.x - currOffset.y + currShift.x - currShift.y;
                _ranges[i].initialise<SD>(min, max - min, margin / ISQRT2, marginWeight, shift, oshift, oshift);
                break;
            case 3 :	// diff (positively sloped diagonal boundaries)
                min = -2 * std::min(currShift.x - _limit.bl.x, _limit.tr.y - currShift.y) + aSlot->origin().x - aSlot->origin().y + currShift.x - currShift.y;
                max = 2 * std::min(_limit.tr.x - currShift.x, currShift.y - _limit.bl.y) + aSlot->origin().x - aSlot->origin().y + currShift.x - currShift.y;
                _len[i] = sb.da - sb.di;
                shift = 0; //currOffset.x - currOffset.y;
                oshift = currOffset.x + currOffset.y + currShift.x + currShift.y;
                _ranges[i].initialise<SD>(min, max - min, margin / ISQRT2, marginWeight, shift, oshift, oshift);
                break;
        }
    }

	_target = aSlot;
    if ((dir & 1) == 0)
    {
        // For LTR, switch and negate x limits.
        _limit.bl.x = -1 * limit.tr.x;
        _limit.tr.x = -1 * limit.bl.x;
    }
    _currOffset = currOffset;
    _currShift = currShift;

	_margin = margin;
	_marginWt = marginWeight;
    
    SlotCollision *c = seg->collisionInfo(aSlot);
    _seqClass = c->seqClass();
	_seqProxClass = c->seqProxClass();
    _seqOrder = c->seqOrder();
}

template <class O>
float sdm(float vi, float va, float mx, float my, O op)
{
    float res = 2 * mx - vi;
    if (op(res, vi + 2 * my))
    {
        res = va + 2 * my;
        if (op(res, 2 * mx - va))
            res = mx + my;
    }
    return res;
}

inline
float sqr(float x) { return x * x; }

// Mark an area with a cost that can vary along the x-axis.
void ShiftCollider::addBox_slope(bool isx, const Rect &box, const BBox &bb, const SlantBox &sb, const Position &org, float weight, float m, bool minright, const Position &offset, int axis)
{
    float a;
    switch (axis) {
        case 0 :
            if (box.bl.y < org.y + bb.height() && box.tr.y > org.y && box.width() > 0)
            {
                a = offset.y;
                if (isx)
                    _ranges[axis].weighted<XY>(box.bl.x, box.width(), weight, offset.x, offset.y, a, m, (minright ? box.tr.x : box.bl.x), 0, false);
                else
                    _ranges[axis].weighted<XY>(box.bl.x, box.width(), weight, offset.x, offset.y, a, 0, 0, m * (a * a + sqr((minright ? box.tr.y : box.bl.y) - offset.y)), false);
            }
            break;
        case 1 :
            if (box.bl.x < org.x + bb.width() && box.tr.x > org.x && box.height() > 0)
            {
                a = offset.x;
                if (isx)
                    _ranges[axis].weighted<XY>(box.bl.y, box.height(), weight, offset.y, offset.x, a, 0, 0, m * (a * a + sqr((minright ? box.tr.x : box.bl.x) - offset.x)), false);
                else
                    _ranges[axis].weighted<XY>(box.bl.y, box.height(), weight, offset.y, offset.x, a, m, (minright ? box.tr.y : box.bl.y), 0, false);
            }
            break;
        case 2 :
            if (box.bl.x - box.tr.y < org.x - org.y + sb.height() && box.tr.x - box.bl.y > org.x - org.y)
            {
                float di = org.x - org.y + sb.di;
                float da = org.x - org.y + sb.da;
                float smax = sdm(di, da, box.tr.x, box.tr.y, std::greater<float>());
                float smin = sdm(da, di, box.bl.x, box.bl.y, std::less<float>());
                if (smin > smax) return;
                float si;
                a = offset.x - offset.y + _currShift.x - _currShift.y;
                if (isx)
                    si = 2 * (minright ? box.tr.x : box.bl.x) - a;
                else
                    si = 2 * (minright ? box.tr.y : box.bl.y) + a;
                _ranges[axis].weighted<SD>(smin, smax - smin, weight / 2, offset.x + offset.y, offset.x - offset.y, a, m / 2, si, 0, true);
            }
            break;
        case 3 :
            if (box.bl.x + box.bl.y < org.x + org.y + sb.width() && box.tr.x + box.tr.y > org.x + org.y)
            {
                float si = org.x + org.y + sb.si;
                float sa = org.x + org.y + sb.sa;
                float dmax = sdm(si, sa, box.tr.x, -box.bl.y, std::greater<float>());
                float dmin = sdm(sa, si, box.bl.x, -box.tr.y, std::less<float>());
                if (dmin > dmax) return;
                float di;
                a = offset.x + offset.y;
                if (isx)
                    di = 2 * (minright ? box.tr.x : box.bl.x) - a;
                else
                    di = 2 * (minright ? box.tr.y : box.bl.y) + a;
                _ranges[axis].weighted<SD>(dmin, dmax - dmin, weight / 2, offset.x - offset.y, offset.x + offset.y, a, m / 2, di, 0, false);
            }
            break;
        default :
            break;
    }
    return;
}

// Mark an area with an absolute cost, making it completely inaccessible.
inline void ShiftCollider::removeBox(const Rect &box, const BBox &bb, const SlantBox &sb, const Position &org, int axis)
{
    switch (axis) {
        case 0 :
            if (box.bl.y < org.y + bb.height() && box.tr.y > org.y && box.width() > 0)
                _ranges[axis].exclude(box.bl.x, box.width());
            break;
        case 1 :
            if (box.bl.x < org.x + bb.width() && box.tr.x > org.x && box.height() > 0)
                _ranges[axis].exclude(box.bl.y, box.height());
            break;
        case 2 :
            if (box.bl.x - box.tr.y < org.x - org.y + sb.height() && box.tr.x - box.bl.y > org.x - org.y && box.width() > 0 && box.height() > 0)
            {
                float di = org.x - org.y + sb.di;
                float da = org.x - org.y + sb.da;
                float smax = sdm(di, da, box.tr.x, box.tr.y, std::greater<float>());
                float smin = sdm(da, di, box.bl.x, box.bl.y, std::less<float>());
                _ranges[axis].exclude(smin, smax - smin);
            }
            break;
        case 3 :
            if (box.bl.x + box.bl.y < org.x + org.y + sb.width() && box.tr.x + box.tr.y > org.x + org.y && box.width() > 0 && box.height() > 0)
            {
                float si = org.x + org.y + sb.si;
                float sa = org.x + org.y + sb.sa;
                float dmax = sdm(si, sa, box.tr.x, -box.bl.y, std::greater<float>());
                float dmin = sdm(sa, si, box.bl.x, -box.tr.y, std::less<float>());
                _ranges[axis].exclude(dmin, dmax - dmin);
            }
            break;
        default :
            break;
    }
    return;
}

// Adjust the movement limits for the target to avoid having it collide
// with the given neighbor slot. Also determine if there is in fact a collision
// between the target and the given slot.
bool ShiftCollider::mergeSlot(Segment *seg, Slot *slot, const Position &currShift,
		bool isAfter,  // slot is logically after _target
		bool sameCluster,
        GR_MAYBE_UNUSED json * const dbgout )
{
    bool isCol = false;
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
    float cmin, cmax;   // target limits
    float vorigin, voorigin;
    const GlyphCache &gc = seg->getFace()->glyphs();
    const unsigned short gid = slot->gid();
    const unsigned short tgid = _target->gid();
    const BBox &bb = gc.getBoundingBBox(gid);
    const SlantBox &sb = gc.getBoundingSlantBox(gid);
    const BBox &tbb = gc.getBoundingBBox(tgid);
    const SlantBox &tsb = gc.getBoundingSlantBox(tgid);

    SlotCollision * cslot = seg->collisionInfo(slot);
    SlotCollision * ctarget = seg->collisionInfo(_target);
    int orderFlags = 0;
    if (sameCluster && _seqClass &&
		((_seqProxClass != 0 && cslot->seqClass() == _seqProxClass)
			|| (_seqProxClass == 0 && cslot->seqClass() == _seqClass)))
		// Force the target glyph to be in the specified direction from the slot we're testing.
        orderFlags = _seqOrder;
    float seq_above_wt = cslot->seqAboveWt();
    float seq_below_wt = cslot->seqBelowWt();
    float seq_valign_wt = cslot->seqValignWt();

    // if isAfter, invert orderFlags for diagonal orders.
    if (isAfter && (orderFlags & 0x3) != 0)        // _target isAfter slot and has LEFTDOWN or RIGHTUP
            orderFlags = orderFlags ^ 0x3;

#if !defined GRAPHITE2_NTRACING
    if (dbgout)
        dbgout->setenv(0, slot);
#endif

    // Process main bounding octabox.
    for (int i = 0; i < 4; ++i)
    {
		//uint16 mMin = (uint16)(_marginMin / (i > 1 ? ISQRT2 : 1.));
        switch (i) {
            case 0 :	// x direction
                vmin = std::max(std::max(bb.xi + sx - tbb.xa, sb.di + sd + tx - tsb.da - td), sb.si + ss + tx - tsb.sa - ts);
                vmax = std::min(std::min(bb.xa + sx - tbb.xi, sb.da + sd + tx - tsb.di - td), sb.sa + ss + tx - tsb.si - ts);
                otmin = tbb.yi + ty;
                otmax = tbb.ya + ty;
                omin = bb.yi + sy;
                omax = bb.ya + sy;
                cmin = _limit.bl.x + _target->origin().x;
                cmax = _limit.tr.x + _target->origin().x + tbb.xa - tbb.xi;
                vorigin = _target->origin().x - ctarget->offset().x;
                break;
            case 1 :	// y direction
                vmin = std::max(std::max(bb.yi + sy - tbb.ya, ty - sb.da - sd + tsb.di + td), sb.si + ss + ty - tsb.sa - ts);
                vmax = std::min(std::min(bb.ya + sy - tbb.yi, ty - sb.di - sd + tsb.da + td), sb.sa + ss + ty - tsb.si - ts);
                otmin = tbb.xi + tx;
                otmax = tbb.xa + tx;
                omin = bb.xi + sx;
                omax = bb.xa + sx;
                cmin = _limit.bl.y + _target->origin().y;
                cmax = _limit.tr.y + _target->origin().y + tbb.ya - tbb.yi;
                vorigin = _target->origin().y - ctarget->offset().y;
                break;
            case 2 :    // sum - moving along the positively-sloped vector, so the boundaries are the
                        // negatively-sloped boundaries.
                vmin = std::max(std::max(sb.si + ss - tsb.sa, 2 * (bb.yi + sy - tbb.ya - ty) + ts), 2 * (bb.xi + sx - tbb.xa - tx) + ts);
                vmax = std::min(std::min(sb.sa + ss - tsb.si, 2 * (bb.ya + sy - tbb.yi - ty) + ts), 2 * (bb.xa + sx - tbb.xi - tx) + ts);
                otmin = tsb.di + td;
                otmax = tsb.da + td;
                omin = sb.di + sd;
                omax = sb.da + sd;
                cmin = _limit.bl.x + _limit.bl.y + _target->origin().x + _target->origin().y; 
                cmax = _limit.tr.x + _limit.tr.y + _target->origin().x + _target->origin().y + tsb.sa - tsb.si;
                vorigin = _target->origin().x + _target->origin().y - ctarget->offset().x - ctarget->offset().y;
                break;
            case 3 :    // diff - moving along the negatively-sloped vector, so the boundaries are the
                        // positively-sloped boundaries.
				// Note that on the d-axis, 
//                if ((orderFlags & 0x3) != 0)     // in d up is down, etc. so invert the flags
//                    orderFlags = orderFlags ^ 0x3;
//                else if ((orderFlags & 0xC) != 0)
//                    orderFlags = orderFlags ^ 0xC;
                vmin = std::max(std::max(sb.di + sd - tsb.da, 2 * (bb.xi + sx - tbb.xa - tx) + td), td - 2 * (bb.ya + sy - tbb.yi - ty));
                vmax = std::min(std::min(sb.da + sd - tsb.di, 2 * (bb.xa + sx - tbb.xi - tx) + td), td - 2 * (bb.yi + sy - tbb.ya - ty));
                otmin = tsb.si + ts;
                otmax = tsb.sa + ts;
                omin = sb.si + ss;
                omax = sb.sa + ss;
                cmin = _limit.bl.x - _limit.tr.y + _target->origin().x - _target->origin().y;
                cmax = _limit.tr.x - _limit.bl.y + _target->origin().x - _target->origin().y + tsb.da - tsb.di;
                vorigin = _target->origin().x - _target->origin().y - ctarget->offset().x + ctarget->offset().y;
                break;
            default :
                continue;
        }
        
#if !defined GRAPHITE2_NTRACING
        if (dbgout)
            dbgout->setenv(1, reinterpret_cast<void *>(-1));
#define DBGTAG(x) if (dbgout) dbgout->setenv(1, reinterpret_cast<void *>(-x));
#else
#define DBGTAG(x)
#endif

        if (orderFlags)
        {
            Position offset = Position(cslot->offset().x + _currShift.x, cslot->offset().y + _currShift.y);
            Position org(tx, ty);
            float xminf = _limit.bl.x + _target->origin().x;
            float xpinf = _limit.tr.x + _target->origin().x;
            float ypinf = _limit.tr.y + _target->origin().y;
            float yminf = _limit.bl.y + _target->origin().y;
            switch (orderFlags) {
                case SlotCollision::SEQ_ORDER_RIGHTUP :
                {
                    float r1Xedge = sx + bb.xa + cslot->seqAboveXoff() - tbb.xa;
                    float r3Xedge = sx + bb.xa + cslot->seqBelowXlim() - tbb.xi;
                    float r2Yedge = sy + 0.5 * (bb.yi + bb.ya + cslot->seqValignHt() - tbb.xi - tbb.xa);
                    
                    // DBGTAG(1x) means the regions are up and right
                    // region 1
                    DBGTAG(11)
                    addBox_slope(true, Rect(Position(xminf, r2Yedge), Position(r1Xedge, ypinf)), tbb, tsb, org, 0, seq_above_wt, true, offset, i);
                    // region 2
                    DBGTAG(12)
                    removeBox(Rect(Position(xminf, yminf), Position(r3Xedge, r2Yedge)), tbb, tsb, org, i);
                    // region 3
                    DBGTAG(13)
                    addBox_slope(true, Rect(Position(r3Xedge, yminf), Position(xpinf, r2Yedge)), tbb, tsb, org, seq_below_wt, 0, true, offset, i);
                    // region 4
                    DBGTAG(14)
                    addBox_slope(false, Rect(Position(sx + bb.xi, r2Yedge), Position(xpinf, r2Yedge + cslot->seqValignHt())), tbb, tsb, org, 0, seq_valign_wt, true, offset, i);
                    // region 5
                    DBGTAG(15)
                    addBox_slope(false, Rect(Position(sx + bb.xi, r2Yedge - cslot->seqValignHt()), Position(xpinf, r2Yedge)),
                                    tbb, tsb, org, 0, seq_valign_wt, false, offset, i);
                    break;
                }
                case SlotCollision::SEQ_ORDER_LEFTDOWN :
                {
                    float r1Xedge = sx + bb.xi - cslot->seqAboveXoff() - tbb.xi;
                    float r3Xedge = sx + bb.xi - cslot->seqBelowXlim() - tbb.xa;
                    float r2Yedge = sy + 0.5 * (bb.yi + bb.ya + cslot->seqValignHt() - tbb.xi - tbb.xa);
                    // DBGTAG(2x) means the regions are up and right
                    // region 1
                    DBGTAG(21)
                    addBox_slope(true, Rect(Position(r1Xedge, yminf), Position(xpinf, r2Yedge)), tbb, tsb, org, 0, seq_above_wt, false, offset, i);
                    // region 2
                    DBGTAG(22)
                    removeBox(Rect(Position(r3Xedge, r2Yedge), Position(xpinf, ypinf)), tbb, tsb, org, i);
                    // region 3
                    DBGTAG(23)
                    addBox_slope(true, Rect(Position(xminf, r2Yedge), Position(r3Xedge, ypinf)), tbb, tsb, org, seq_below_wt, 0, true, offset, i);
                    // region 4
                    DBGTAG(24)
                    addBox_slope(false, Rect(Position(xminf, r2Yedge), Position(sx + bb.xa, r2Yedge + cslot->seqValignHt())),
                                    tbb, tsb, org, 0, seq_valign_wt, true, offset, i);
                    // region 5
                    DBGTAG(25)
                    addBox_slope(false, Rect(Position(xminf, r2Yedge - cslot->seqValignHt()),
                                    Position(sx + bb.xa, r2Yedge)), tbb, tsb, org, 0, seq_valign_wt, false, offset, i);
                    break;
                }
                case SlotCollision::SEQ_ORDER_NOABOVE : // enforce neighboring glyph being above
                    DBGTAG(31);
                    removeBox(Rect(Position(sx + bb.xi - tbb.xa, sy + bb.ya), Position(sx + bb.xa - tbb.xi, _target->origin().y + _limit.tr.y)), tbb, tsb, org, i);
                    break;
                case SlotCollision::SEQ_ORDER_NOBELOW :	// enforce neighboring glyph being below
                    DBGTAG(32);
                    removeBox(Rect(Position(sx + bb.xi - tbb.xa, _target->origin().y + _limit.bl.y), Position(sx + bb.xa - tbb.xi, sy + bb.yi)), tbb, tsb, org, i);
                    break;
                default :
                    break;
            }
        }

        // if ((vmin < cmin - m && vmax < cmin - m) || (vmin > cmax + m && vmax > cmax + m)
        //    // or it is offset in the opposite dimension:
        //    || (omin < otmin - m && omax < otmin - m) || (omin > otmax + m && omax > otmax + m))
        if (vmax < cmin - _margin || vmin > cmax + _margin || omax < otmin - _margin || omin > otmax + _margin)
            continue;
#if 0
		if (seg->collisionInfo(_target)->canScrape(i) && (omax < otmin + _margin || omin > otmax - _margin))
		{
			_scraping[i] = true;
			continue;
		}
#endif

        // Process sub-boxes that are defined for this glyph.
        // We only need to do this if there was in fact a collision with the main octabox.
        uint8 numsub = gc.numSubBounds(gid);
        if (numsub > 0)
        {
            bool anyhits = false;
            for (int j = 0; j < numsub; ++j)
            {
                const BBox &sbb = gc.getSubBoundingBBox(gid, j);
                const SlantBox &ssb = gc.getSubBoundingSlantBox(gid, j);
                switch (i) {
                    case 0 :    // x
                        vmin = std::max(std::max(sbb.xi + sx - tbb.xa, ssb.di + sd + tx - tsb.da - td), ssb.si + ss + tx - tsb.sa - ts);
                        vmax = std::min(std::min(sbb.xa + sx - tbb.xi, ssb.da + sd + tx - tsb.di - td), ssb.sa + ss + tx - tsb.si - ts);
                        omin = sbb.yi + sy;
                        omax = sbb.ya + sy;
                        break;
                    case 1 :    // y
                        vmin = std::max(std::max(sbb.yi + sy - tbb.ya, ty - ssb.da - sd + tsb.di + td), ssb.si + ss + ty - tsb.sa - ts);
                        vmax = std::min(std::min(sbb.ya + sy - tbb.yi, ty - ssb.di - sd + tsb.da + td), ssb.sa + ss + ty - tsb.si - ts);
                        omin = sbb.xi + sx;
                        omax = sbb.xa + sx;
                        break;
                    case 2 :    // sum
                        vmin = std::max(std::max(ssb.si + ss - tsb.sa, 2 * (sbb.yi + sy - tbb.ya - ty) + ts), 2 * (sbb.xi + sx - tbb.xa - tx) + ts);
                        vmax = std::min(std::min(ssb.sa + ss - tsb.si, 2 * (sbb.ya + sy - tbb.yi - ty) + ts), 2 * (sbb.xa + sx - tbb.xi - tx) + ts);
                        omin = ssb.di + sd;
                        omax = ssb.da + sd;
                        break;
                    case 3 :    // diff
                        vmin = std::max(std::max(ssb.di + sd - tsb.da, 2 * (sbb.xi + sx - tbb.xa - tx) + td), td - 2 * (sbb.ya + sy - tbb.yi - ty));
                        vmax = std::min(std::min(ssb.da + sd - tsb.di, 2 * (sbb.xa + sx - tbb.xi - tx) + td), td - 2 * (sbb.yi + sy - tbb.ya - ty));
                        omin = ssb.si + ss;
                        omax = ssb.sa + ss;
                        break;
                }
                // if ((vmin < cmin - m && vmax < cmin - m) || (vmin > cmax + m && vmax > cmax + m)
                //     		|| (omin < otmin - m && omax < otmin - m) || (omin > otmax + m && omax > otmax + m))
                if (vmax < cmin - _margin || vmin > cmax + _margin || omax < otmin - _margin || omin > otmax + _margin)
                    continue;
#if 0
				if (seg->collisionInfo(_target)->canScrape(i) && (omax < otmin + _margin || omin > otmax - _margin))
				{
					_scraping[i] = true;
					continue;
				}
#endif

#if !defined GRAPHITE2_NTRACING
                if (dbgout)
                    dbgout->setenv(1, reinterpret_cast<void *>(j));
#endif
                _ranges[i].exclude_with_margins(vmin, vmax - vmin, vorigin, i);
                anyhits = true;
            }
            if (anyhits)
                isCol = true;
        }
        else // no sub-boxes
        {
#if !defined GRAPHITE2_NTRACING
                if (dbgout)
                    dbgout->setenv(1, reinterpret_cast<void *>(-1));
#endif
            isCol = true;
            _ranges[i].exclude_with_margins(vmin, vmax - vmin, vorigin, i);

        }
    }
    
    if (cslot && cslot->exclGlyph() > 0)
    {
        // Set up the bogus slot representing the exclusion glyph.
        Slot *exclSlot = seg->newSlot();
        exclSlot->setGlyph(seg, cslot->exclGlyph());
        Position exclOrigin(slot->origin() + cslot->exclOffset());
        exclSlot->origin(exclOrigin);
        isCol |= mergeSlot(seg, exclSlot, currShift, isAfter, sameCluster, dbgout );
        seg->freeSlot(exclSlot);
    }
        
    return isCol;
    
}   // end of ShiftCollider::mergeSlot


// Figure out where to move the target glyph to, and return the amount to shift by.
Position ShiftCollider::resolve(Segment *seg, bool &isCol, GR_MAYBE_UNUSED json * const dbgout)
{
    float tbase, tval;
    float totalCost = (float)(std::numeric_limits<float>::max() / 2.);
    Position resultPos = Position(0, 0);
    Position currOffset = seg->collisionInfo(_target)->offset();
#if !defined GRAPHITE2_NTRACING
	int bestAxis = -1;
    if (dbgout)
    {
		outputJsonDbgStartSlot(dbgout, seg);
        *dbgout << "vectors" << json::array;
    }
#endif
    isCol = true;
    for (int i = 0; i < 4; ++i)
    {
        float bestCost = -1;
        float bestPos;
        // Calculate the margin depending on whether we are moving diagonally or not:
        switch (i) {
            case 0 :	// x direction
                tbase = _target->origin().x;    // The best place to be for the glyph, its anchor
                tval = currOffset.x;
                break;
            case 1 :	// y direction
                tbase = _target->origin().y;
                tval = currOffset.y;
                break;
            case 2 :	// sum (negatively-sloped diagonals)
                tbase = _target->origin().x + _target->origin().y;
                tval = currOffset.x + currOffset.y;
                break;
            case 3 :	// diff (positively-sloped diagonals)
                tbase = _target->origin().x - _target->origin().y;
                tval = currOffset.x - currOffset.y;
                break;
        }
        Position testp;
        bestPos = _ranges[i].closest(tbase - tval, bestCost) - tbase;     // returns absolute, convert to shift.
#if !defined GRAPHITE2_NTRACING
        if (dbgout)
            outputJsonDbgOneVector(dbgout, seg, i, tbase, bestCost, bestPos) ;
#endif
        if (bestCost >= 0.0)
        {
            isCol = false;
            switch (i) {
                case 0 : testp = Position(bestPos, _currShift.y); break;
                case 1 : testp = Position(_currShift.x, bestPos); break;
                case 2 : testp = Position(0.5 * (bestPos + _currShift.x - _currShift.y), 0.5 * (bestPos - _currShift.x + _currShift.y)); break;
                case 3 : testp = Position(0.5 * (bestPos + _currShift.x + _currShift.y), 0.5 * (_currShift.x + _currShift.y - bestPos)); break;
            }
            if (bestCost < totalCost)
            {
                totalCost = bestCost;
                resultPos = testp;
#if !defined GRAPHITE2_NTRACING
                bestAxis = i;
#endif
            }
        }
    }  // end of loop over 4 directions

#if !defined GRAPHITE2_NTRACING
    if (dbgout)
        outputJsonDbgEndSlot(dbgout, resultPos, bestAxis, isCol);
#endif

    return resultPos;

}   // end of ShiftCollider::resolve


#if !defined GRAPHITE2_NTRACING

void ShiftCollider::outputJsonDbg(json * const dbgout, Segment *seg, int axis)
{
    int axisMax = axis;
    if (axis < 0) // output all axes
    {
        *dbgout << "gid" << _target->gid()
            << "limit" << _limit
            << "target" << json::object
                << "origin" << _target->origin()
                << "margin" << _margin
                << "bbox" << seg->theGlyphBBoxTemporary(_target->gid())
                << "slantbox" << seg->getFace()->glyphs().slant(_target->gid())
                << json::close; // target object
        *dbgout << "ranges" << json::array;
        axis = 0;
        axisMax = 3;
    }
    for (int iAxis = axis; iAxis <= axisMax; ++iAxis)
    {
        *dbgout << json::flat << json::array << _ranges[iAxis].position();
        for (Zones::const_iterator s = _ranges[iAxis].begin(), e = _ranges[iAxis].end(); s != e; ++s)
            *dbgout << json::flat << json::array 
                        << Position(s->x, s->xm) << s->sm << s->smx << s->c
                    << json::close;
        *dbgout << json::close;
    }
    if (axis < axisMax) // looped through the _ranges array for all axes
        *dbgout << json::close; // ranges array
}

void ShiftCollider::outputJsonDbgStartSlot(json * const dbgout, Segment *seg)
{
        *dbgout << json::object // slot - not closed till the end of the caller method
                << "slot" << objectid(dslot(seg, _target))
				<< "gid" << _target->gid()
                << "limit" << _limit
                << "target" << json::object
                    << "origin" << _target->origin()
                    << "currShift" << _currShift
                    << "currOffset" << seg->collisionInfo(_target)->offset()
                    << "bbox" << seg->theGlyphBBoxTemporary(_target->gid())
                    << "slantBox" << seg->getFace()->glyphs().slant(_target->gid())
                    << "fix" << "shift";
        *dbgout     << json::close; // target object
}

void ShiftCollider::outputJsonDbgEndSlot(GR_MAYBE_UNUSED json * const dbgout,
	 Position resultPos, int bestAxis, bool isCol)
{
    *dbgout << json::close // vectors array
    << "result" << resultPos
	//<< "scraping" << _scraping[bestAxis]
	<< "bestAxis" << bestAxis
    << "stillBad" << isCol
    << json::close; // slot object
}

void ShiftCollider::outputJsonDbgOneVector(json * const dbgout, Segment *seg, int axis,
	float tleft, float bestCost, float bestVal) 
{
	const char * label;
	switch (axis)
	{
		case 0:	label = "x";			break;
		case 1:	label = "y";			break;
		case 2:	label = "sum (NE-SW)";	break;
		case 3:	label = "diff (NW-SE)";	break;
		default: label = "???";			break;
	}

	*dbgout << json::object // vector
		<< "direction" << label
		<< "targetMin" << tleft;
            
	outputJsonDbgRemovals(dbgout, axis, seg);
    	
    *dbgout << "ranges";
    outputJsonDbg(dbgout, seg, axis);

    *dbgout << "bestCost" << bestCost
        << "bestVal" << bestVal
        << json::close; // vectors object
}

void ShiftCollider::outputJsonDbgRemovals(json * const dbgout, int axis, Segment *seg)
{
    *dbgout << "removals" << json::array;
    _ranges[axis].jsonDbgOut(seg);
    *dbgout << json::close; // removals array
}

#endif // !defined GRAPHITE2_NTRACING


////    KERN-COLLIDER    ////

// Return the left edge of the glyph at height y, taking any slant box into account.
static float get_left(Segment *seg, const Slot *s, const Position &shift, float y, float width)
{
    const GlyphCache &gc = seg->getFace()->glyphs();
    unsigned short gid = s->gid();
    float sx = s->origin().x + shift.x;
    float sy = s->origin().y + shift.y;
    uint8 numsub = gc.numSubBounds(gid);
    float res = (float)1e38;

    if (numsub > 0)
    {
        for (int i = 0; i < numsub; ++i)
        {
            const BBox &sbb = gc.getSubBoundingBBox(gid, i);
            const SlantBox &ssb = gc.getSubBoundingSlantBox(gid, i);
            if (sy + sbb.yi > y + width / 2 || sy + sbb.ya < y - width / 2)
                continue;
            float x = sx + sbb.xi;
            if (x < res)
            {
                x = std::max(sx - sy + ssb.di + y, std::max(sx + sy + ssb.si - y, x));
                if (x < res)
                    res = x;
            }
        }
    }
    else
    {
        const BBox &bb = gc.getBoundingBBox(gid);
        const SlantBox &sb = gc.getBoundingSlantBox(gid);
        res = std::max(sx + bb.xi, std::max(sx - sy + sb.di + y, sx + sy + sb.si - y));
    }
    return res;
}

// Return the right edge of the glyph at height y, taking any slant boxes into account.
static float get_right(Segment *seg, const Slot *s, const Position &shift, float y, float width)
{
    const GlyphCache &gc = seg->getFace()->glyphs();
    unsigned short gid = s->gid();
    float sx = s->origin().x + shift.x;
    float sy = s->origin().y + shift.y;
    uint8 numsub = gc.numSubBounds(gid);
    float res = (float)-1e38;

    if (numsub > 0)
    {
        for (int i = 0; i < numsub; ++i)
        {
            const BBox &sbb = gc.getSubBoundingBBox(gid, i);
            const SlantBox &ssb = gc.getSubBoundingSlantBox(gid, i);
            if (sy + sbb.yi > y + width / 2 || sy + sbb.ya < y - width / 2)
                continue;
            float x = sx + sbb.xa;
            if (x > res)
            {
                x = std::min(sx - sy + ssb.da + y, std::min(sx + sy + ssb.sa - y, x));
                if (x > res)
                    res = x;
            }
        }
    }
    else
    {
        const BBox &bb = gc.getBoundingBBox(gid);
        const SlantBox &sb = gc.getBoundingSlantBox(gid);
        res = std::min(sx + bb.xa, std::min(sx - sy + sb.da + y, sx + sy + sb.sa - y));
    }
    return res;
}

     

void KernCollider::initSlot(Segment *seg, Slot *aSlot, const Rect &limit, float margin,
    const Position &currShift, const Position &offsetPrev, int dir, GR_MAYBE_UNUSED json * const dbgout)
{
    const GlyphCache &gc = seg->getFace()->glyphs();
    const Slot *base = aSlot;
    // const Slot *last = aSlot;
    const Slot *s;
    unsigned int maxid = aSlot->index();
    float sliceWidth;
    while (base->attachedTo())
        base = base->attachedTo();
    if (margin < 10) margin = 10;

    _limit = limit;
    _offsetPrev = offsetPrev; // kern from a previous pass
    
    // Calculate the height of the glyph and how many horizontal slices to use.
    _maxy = (float)-1e38;
    _miny = (float)1e38;
    _xbound = (dir & 1) ? (float)1e38 : (float)-1e38;
    _othermax = -_xbound;
    for (s = base; s; s = s->nextInCluster(s))
    {
        SlotCollision *c = seg->collisionInfo(s);
        const BBox &bs = gc.getBoundingBBox(s->gid());
        if (s->index() > maxid)
        {
            // last = s;
            maxid = s->index();
        }
        float y = s->origin().y + c->shift().y;
        _maxy = std::max(_maxy, y + bs.ya);
        _miny = std::min(_miny, y + bs.yi);
    }
    _numSlices = int((_maxy - _miny + 2) / margin + 1.);  // +2 helps with rounding errors
    sliceWidth = (_maxy - _miny + 2) / _numSlices;
    _edges.clear();
    _edges.insert(_edges.begin(), _numSlices, (dir & 1) ? 1e38 : -1e38);
        
#if !defined GRAPHITE2_NTRACING
    // Debugging
    _seg = seg;
    _slotNear.clear();
    _slotNear.insert(_slotNear.begin(), _numSlices, NULL);
    _nearEdges.clear();
    _nearEdges.insert(_nearEdges.begin(), _numSlices, (dir & 1) ? -1e38 : +1e38);
#endif
    
    // Determine the trailing edge of each slice (ie, left edge for a RTL glyph).
    for (s = base; s; s = s->nextInCluster(s))
    {
        SlotCollision *c = seg->collisionInfo(s);
        const BBox &bs = gc.getBoundingBBox(s->gid());
        float x = s->origin().x + c->shift().x + (dir & 1 ? bs.xi : bs.xa);
        // Loop over slices.
        // Note smin might not be zero if glyph s is not at the bottom of the cluster; similarly for smax.
        int smin = std::max(0, int((s->origin().y + c->shift().y + bs.yi - _miny + 1) / (_maxy - _miny + 2) * _numSlices));
        int smax = std::min(_numSlices - 1, int((s->origin().y + c->shift().y + bs.ya - _miny + 1) / (_maxy - _miny + 2) * _numSlices + 1));
        for (int i = smin; i <= smax; ++i)
        {
            float t;
            float y = _miny - 1 + (i + .5) * sliceWidth; // vertical center of slice
            if ((dir & 1) && x < _edges[i])
            {
                t = get_left(seg, s, currShift, y, sliceWidth);
                if (t < _edges[i])
                {
                    _edges[i] = t;
                    if (t < _xbound)
                        _xbound = t;
                }
            }
            else if (!(dir & 1) && x > _edges[i])
            {
                t = get_right(seg, s, currShift, y, sliceWidth);
                if (t > _edges[i])
                {
                    _edges[i] = t;
                    if (t > _xbound)
                        _xbound = t;
                }
            }
        }
    }
    _mingap = (float)1e38;
    _target = aSlot;
    _margin = margin;
    _currShift = currShift;
    
}   // end of KernCollider::initSlot


// Determine how much the target slot needs to kern away from the given slot.
// In other words, merge information from given slot's position with what the target slot knows
// about how it can kern.
// Return false if we know there is no collision, true if we think there might be one.
bool KernCollider::mergeSlot(Segment *seg, Slot *slot, const Position &currShift, float currSpace, int dir, GR_MAYBE_UNUSED json * const dbgout)
{
    const Rect &bb = seg->theGlyphBBoxTemporary(slot->gid());
    const float sx = slot->origin().x + currShift.x;
    const float sy = slot->origin().y + currShift.y;
    int smin = std::max(0, int((sy + bb.bl.y - _miny + 1) / (_maxy - _miny + 2) * _numSlices));
    int smax = std::min(_numSlices - 1, int((sy + bb.tr.y - _miny + 1) / (_maxy - _miny + 2) * _numSlices + 1));
    float sliceWidth = (_maxy - _miny + 2) / _numSlices;
    bool collides = false;

    if (dir & 1)
    {
        float x = sx + bb.tr.x;
        if (x < _xbound - _mingap) // this isn't going to reduce _mingap so skip
            return false;
        for (int i = smin; i <= smax; ++i)
        {
            float t;
            float y = (float)(_miny - 1 + (i + .5) * sliceWidth);  // vertical center of slice
            if (x > _edges[i] - _mingap)
            {
                float m = get_right(seg, slot, currShift, y, sliceWidth) + currSpace;
                if (_othermax < m) _othermax = m;
                t = _edges[i] - m;
                // Check slices above and below (if any).
                if (i < _numSlices - 1) t = std::min(t, _edges[i+1] - m);
                if (i > 0) t = std::min(t, _edges[i-1] - m);
                if (t < _mingap)
                {
                    _mingap = t;
                    collides = true;
                }
#if !defined GRAPHITE2_NTRACING
                // Debugging - remember the closest neighboring edge for this slice.
                if (m > _nearEdges[i])
                {
                    _slotNear[i] = slot;
                    _nearEdges[i] = m;
                }
#endif
            }
        }
    }
    else
    {
        float x = sx + bb.bl.x;
        if (x > _xbound + _mingap + currSpace)
            return false;
        for (int i = smin; i < smax; ++i)
        {
            float t;
            float y = (float)(_miny - 1 + (i + .5) * sliceWidth);  // vertical center of slice
            if (x < _edges[i] + _mingap)
            {
                float m = get_left(seg, slot, currShift, y, sliceWidth) + currSpace;
                if (m > _othermax) _othermax = m;
                t = m - _edges[i];
                if (i < _numSlices - 1) t = std::min(t, m - _edges[i+1]);
                if (i > 0) t = std::min(t, m - _edges[i-1]);
                if (t < _mingap)
                {
                    _mingap = t;
                    collides = true;
                }

#if !defined GRAPHITE2_NTRACING
                // Debugging - remember the closest neighboring edge for this slice.
                if (m < _nearEdges[1])
                {
                    _slotNear[i] = slot;
                    _nearEdges[i] = m;
                }
#endif
            }
        }
    }
    return collides;   // note that true is not a necessarily reliable value
    
}   // end of KernCollider::mergeSlot


// Return the amount to kern by.
// TODO: do we need to make use of marginMin here? Probably not.
Position KernCollider::resolve(GR_MAYBE_UNUSED Segment *seg, GR_MAYBE_UNUSED Slot *slot, int dir, float margin,
        GR_MAYBE_UNUSED json * const dbgout)
{
    float resultNeeded = (1 - 2 * (dir & 1)) * (_mingap - margin);
    float result = min(_limit.tr.x - _offsetPrev.x, max(resultNeeded, _limit.bl.x - _offsetPrev.x));
//    const SlotCollision *cslot = seg->collisionInfo(slot);
    // PUT SOMETHING IN HERE TO HANDLE ORDER-ENFORCEMENT
//    if (cslot->flags() & SlotCollision::COLL_OVERLAP && _othermax - _xbound - _mingap > -1 * cslot->maxOverlap())
//        resultNeeded = (1 - 2 * (dir & 1)) * (_xbound - _othermax + cslot->maxOverlap() + margin);
    

#if !defined GRAPHITE2_NTRACING
    float sliceWidth = (_maxy - _miny + 2) / _numSlices; // copied from above
    if (dbgout)
    {
        *dbgout << json::object // slot
                << "slot" << objectid(dslot(seg, _target))
				<< "gid" << _target->gid()
                << "margin" << _margin
                << "limit" << _limit
                << "target" << json::object
                    << "origin" << _target->origin()
                    //<< "currShift" << _currShift
                    << "offsetPrev" << _offsetPrev
                    << "bbox" << seg->theGlyphBBoxTemporary(_target->gid())
                    << "slantBox" << seg->getFace()->glyphs().slant(_target->gid())
                    << "fix" << "kern"
                    << "slices" << _numSlices
                    << "sliceWidth" << sliceWidth
                    << json::close; // target object
        
        *dbgout << "slices" << json::array;
        for (int is = 0; is < _numSlices; is++)
        {
            *dbgout << json::flat << json::object 
                << "i" << is 
                << "targetEdge" << _edges[is]
                << "neighbor" << objectid(dslot(seg, _slotNear[is]))
                << "nearEdge" << _nearEdges[is] 
                << json::close;
        }
        *dbgout << json::close; // slices array
            
        *dbgout
            << "xbound" << _xbound
            << "minGap" << _mingap
            << "needed" << resultNeeded
            << "result" << result
            << "stillBad" << (result != resultNeeded)
            << json::close; // slot object
    }
#endif

    return Position(result, 0.);
    
}   // end of KernCollider::resolve


////    SLOT-COLLISION    ////

// Initialize the collision attributes for the given slot.
SlotCollision::SlotCollision(Segment *seg, Slot *slot)
{
    initFromSlot(seg, slot);
}

void SlotCollision::initFromSlot(Segment *seg, Slot *slot)
{
    // Initialize slot attributes from glyph attributes.
	// The order here must match the order in the grcompiler code, 
	// GrcSymbolTable::AssignInternalGlyphAttrIDs.
    uint16 gid = slot->gid();
    uint16 aCol = seg->silf()->aCollision(); // flags attr ID
    _flags = seg->glyphAttr(gid, aCol);
    _status = _flags;
    _limit = Rect(Position(seg->glyphAttr(gid, aCol+1), seg->glyphAttr(gid, aCol+2)),
                  Position(seg->glyphAttr(gid, aCol+3), seg->glyphAttr(gid, aCol+4)));
    _margin = seg->glyphAttr(gid, aCol+5);
    _marginWt = seg->glyphAttr(gid, aCol+6);

    // TODO: do we want to initialize collision.exclude stuff from the glyph attributes,
    // or make GDL do it explicitly?
//  _exclGlyph = seg->glyphAttr(gid, aCol+7);
//  _exclOffset = Position(seg->glyphAttr(gid, aCol+8), seg->glyphAttr(gid, aCol+9));
    _exclGlyph = 0;
    _exclOffset = Position(0, 0);

    _seqClass = seg->glyphAttr(gid, aCol+10);
	_seqProxClass = seg->glyphAttr(gid, aCol+11);
    _seqOrder = seg->glyphAttr(gid, aCol+12);
	_seqAboveXoff = seg->glyphAttr(gid, aCol+13);
	_seqAboveWt = seg->glyphAttr(gid, aCol+14);
	_seqBelowXlim = seg->glyphAttr(gid, aCol+15);
	_seqBelowWt = seg->glyphAttr(gid, aCol+16);
	_seqValignHt = seg->glyphAttr(gid, aCol+17);
	_seqValignWt = seg->glyphAttr(gid, aCol+18);    

	//_canScrape[0] = _canScrape[1] = _canScrape[2] = _canScrape[3] = true;
}

float SlotCollision::getKern(int dir) const
{
    if ((_flags & SlotCollision::COLL_KERN) != 0)
        return float(_shift.x * ((dir & 1) ? -1 : 1));
    else
    	return 0;
}

