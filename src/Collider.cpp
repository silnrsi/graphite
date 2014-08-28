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

#define ISQRT2 0.707106781

using namespace graphite2;

void Collider::initSlot(Slot *aSlot, const Rect &limit)
{
    int i;
    for (i = 0; i < 4; ++i)
        _colQueues[i].clear();
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
    float vmin, vmax, vbase;
    float cmin, cmax;
    const GlyphCache &gc = seg->getFace()->glyphs();
    unsigned short gid = slot->gid();
    for (int i = 0; i < 4; ++i)
    {
        switch (i) {
            case 0 :
                vbase = slot->origin().x;
                vmin = gc.getBoundingMetric(gid, 0) + vbase;
                vmax = gc.getBoundingMetric(gid, 2) + vbase;
                cmin = _limit.bl.x;
                cmax = _limit.tr.x;
                break;
            case 1 :
                vbase = slot->origin().y;
                vmin = gc.getBoundingMetric(gid, 1) + vbase;
                vmax = gc.getBoundingMetric(gid, 3) + vbase;
                cmin = _limit.bl.y;
                cmax = _limit.tr.y;
                break;
            case 2 :
                vbase = slot->origin().x + slot->origin().y;
                vmin = gc.getBoundingMetric(gid, 4) + vbase;
                vmax = gc.getBoundingMetric(gid, 6) + vbase;
                cmin = _limit.bl.x + _limit.bl.y;
                cmax = _limit.tr.x + _limit.tr.y;
                break;
            case 3 :
                vbase = slot->origin().x - slot->origin().y;
                vmin = gc.getBoundingMetric(gid, 5) + vbase;
                vmax = gc.getBoundingMetric(gid, 7) + vbase;
                cmin = _limit.bl.x - _limit.tr.y;
                cmax = _limit.tr.x - _limit.bl.y;
                break;
            default :
                continue;
        }
        if ((vmin < cmin && vmax < cmin) || (vmin > cmax && vmax > cmax))
        {
            isCol = false;
            continue;
        }
        uint8 numsub = gc.numSubBounds(slot->gid());
        if (numsub > 0)
        {
            bool anyhits = false;
            for (int j = 0; j < numsub; ++j)
            {
                switch (i) {
                    case 0 :
                        vmin = gc.getSubBoundingMetric(gid, j, 0) + vbase;
                        vmax = gc.getSubBoundingMetric(gid, j, 2) + vbase;
                        break;
                    case 1 :
                        vmin = gc.getSubBoundingMetric(gid, j, 1) + vbase;
                        vmax = gc.getSubBoundingMetric(gid, j, 3) + vbase;
                        break;
                    case 2 :
                        vmin = gc.getSubBoundingMetric(gid, j, 4) + vbase;
                        vmax = gc.getSubBoundingMetric(gid, j, 6) + vbase;
                        break;
                    case 3 :
                        vmin = gc.getSubBoundingMetric(gid, j, 5) + vbase;
                        vmax = gc.getSubBoundingMetric(gid, j, 7) + vbase;
                        break;
                }
                if ((vmin < cmin && vmax < cmin) || (vmin > cmax && vmax > cmax))
                    continue;
                ivfpairs ind = std::upper_bound(_colQueues[i].begin(), _colQueues[i].end(), vmin, cmpfpair);
                _colQueues[i].insert(ind, fpair(vmin, vmax));
                anyhits = true;
            }
            if (!anyhits)
                isCol = false;
        }
        else
        {
            // need code here and above to keep the n best pairs rather than all of them.
            ivfpairs ind = std::upper_bound(_colQueues[i].begin(), _colQueues[i].end(), vmin, cmpfpair);
            _colQueues[i].insert(ind, fpair(vmin, vmax));
        }
    }
    return isCol;
}

float Collider::cost(GR_MAYBE_UNUSED float distance, float oover, float tover)
{
    if (tover < .0001) return 0.;
    return tover + oover;
}

void Collider::testloc(float start, ivfpairs ind, ivfpairs begin, ivfpairs end,
                        float torig, float tlen, uint16 margin, float &bestc, float &bestd)
{
    float oover = 0.;
    float tover = 0.;
    ivfpairs kind, kend;
    for (kend = ind; kend != end; ++kend)
    {
        if (kend->first > start + tlen)
            break;
    }
    for (kind = begin; kind != kend; ++kind)
    {
        if (kind->second < start)
            continue;
        float left = (kind->first < start + margin) ? start + margin : kind->first;
        float right = (kind->second > start + tlen + margin) ? start + tlen + margin : kind->second;
        tover += (right - left) / tlen;
        oover += (right - left) / (kind->second - kind->first);
    }
    float c = cost(fabs(torig - start), oover, tover);
    if (c < bestc || (c == bestc && fabs(start - torig) < fabs(bestd)))
    {
        bestc = c;
        bestd = start - torig;
    }
}

Position Collider::resolve(Segment *seg, bool &isCol, const Position &currshift)
{
    const GlyphCache &gc = seg->getFace()->glyphs();
    int gid = _base->gid();
    int margin = seg->collisionInfo(_base)->margin();
    float tlen, torig;
    ivfpairs jind, jend;
    float totalc = std::numeric_limits<float>::max();
    float totald = totalc;
    Position totalp;
    float cmax, cmin;
    for (int i = 0; i < 4; ++i)
    {
        float bestc = std::numeric_limits<float>::max();
        float bestd = bestc;
        switch (i) {
            case 0 :
                tlen = gc.getBoundingMetric(gid, 2) - gc.getBoundingMetric(gid, 0);
                torig = _base->origin().x + currshift.x;
                cmin = _limit.bl.x;
                cmax = _limit.tr.x;
                break;
            case 1 :
                tlen = gc.getBoundingMetric(gid, 3) - gc.getBoundingMetric(gid, 1);
                torig = _base->origin().y + currshift.y;
                cmin = _limit.bl.y;
                cmax = _limit.tr.y;
                break;
            case 2 :
                tlen = gc.getBoundingMetric(gid, 6) - gc.getBoundingMetric(gid, 4);
                torig = _base->origin().x + _base->origin().y + currshift.x + currshift.y;
                cmin = _limit.bl.x + _limit.bl.y;
                cmax = _limit.tr.x + _limit.tr.y;
                break;
            case 3 :
                tlen = gc.getBoundingMetric(gid, 7) - gc.getBoundingMetric(gid, 5);
                torig = _base->origin().x - _base->origin().y + currshift.x - currshift.y;
                cmin = _limit.bl.x - _limit.bl.y;
                cmax = _limit.tr.x - _limit.tr.y;
                break;
        }
        // O(N^2) :(
        for (jind = _colQueues[i].begin(), jend = _colQueues[i].end(); jind != jend; ++jind)
        {
            float p = jind->first - tlen - margin;
            if (p > cmin && p < cmax)
                testloc(p, jind, _colQueues[i].begin(), jend, torig, tlen, margin, bestc, bestd);
            p = jind->second + margin;
            if (p > cmin && p < cmax)
                testloc(p, jind, _colQueues[i].begin(), jend, torig, tlen, margin, bestc, bestd);
        }
        if (bestc < totalc || (bestc == totalc && fabs(bestd) * (i > 1 ? ISQRT2 : 1.) < totald))
        {
            totalc = bestc;
            totald = fabs(bestd) * (i > 1 ? ISQRT2 : 1.);
            switch (i) {
                case 0 : totalp = Position(torig + bestd, 0.); break;
                case 1 : totalp = Position(0., torig + bestd); break;
                case 2 : totalp = Position(_base->origin().x + bestd, _base->origin().y + bestd); break;
                case 3 : totalp = Position(_base->origin().x + bestd, _base->origin().y - bestd); break;
            }
        }
    }
    isCol = (totalc > 0.);
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

