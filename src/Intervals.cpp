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

#include "inc/Intervals.h"
#include "inc/Segment.h"
#include "inc/Slot.h"
#include "inc/debug.h"

using namespace graphite2;

#include <cmath>

#if 0
///  INTERVAL SET  ///

// Add this interval to the list of possible range(s), merging elements of the list as necessary.
// Eg, if the ranges are [{100..200), (500..700)], adding (150..300) will result in
// [{100..300), (500..700)].
void IntervalSet::add(IntervalSet::tpair interval)
{
    IntervalSet::ivtpair slast = _v.end();
    for (IntervalSet::ivtpair s = _v.begin(), e = _v.end(); s != e; ++s)
    {
        if (s->first > interval.second)
            break;          // We have passed the end of ranges. No more to be done
        if (s->second < interval.first)
            continue;       // Skip until we find something
        if (slast != e)
        {                   // We've already modified something, so merge slots
            s->first = slast->first;
            _v.erase(slast);
            --s;
            e = _v.end();
        }                   // Update current slot to incorporate interval
        if (s->second < interval.second)
            s->second = interval.second;
        if (s->first > interval.first)
            s->first = interval.first;
        slast = s;
    }
    if (slast == _v.end())  // interval not already added
        append(interval);    // The added range is off to the far right
}

// Remove the range represented by the interval from the possible range(s), splitting elements of
// the list as necessary.
// Eg, if the ranges are [{100..200), (500..700)], removing (550..600) will result in
// [{100..300), (500..550), (600..700)].
void IntervalSet::remove(IntervalSet::tpair interval)
{
    for (IntervalSet::ivtpair s = _v.begin(), e = _v.end(); s != e; ++s)
    {
        if (s->second < interval.first - _len)
            continue;
        if (s->first > interval.second)
            break;

        if (s->first >= interval.first - _len && s->second <= interval.second)
        {
            _v.erase(s);
            --s;
            e = _v.end();
            continue;
        }
        if (s->first <= interval.first - _len && s->second >= interval.second)
        {                   // Need to split this range since overlap on both sides
            s = _v.insert(s, tpair(s->first, interval.first - _len));
            ++s;
            s->first = interval.second;
            e = _v.end(); //TODO: ????
        }
        if (s->first < interval.second && s->first > interval.first - _len)
            s->first = interval.second;
        if (s->second < interval.second && s->second > interval.first - _len)
            s->second = interval.first - _len;
    }
}


float IntervalSet::findBestWithMarginAndLimits(float val, float margin, float minMargin, int &foundGood)
{
    float res = std::numeric_limits<float>::max();
    float sres = res;
    float lres = res;
    for (IntervalSet::ivtpair s = _v.begin(), e = _v.end(); s != e; ++s)
    {
        if (foundGood && s->first > res)
            break;
        float w = (s->second - s->first) / 2;
        float t = (s->second + s->first) / 2;
        if (w < 0)
            continue;
        else if (w < minMargin)
        {
            if (fabs(t - val) < lres)
                lres = t;
        }
        else if (w < margin)
        {
            if (fabs(t - val) < sres)
                sres = t;
            if (foundGood < 1)
                foundGood = 1;
        }
        else
        {
            float left = s->first + margin;
            float right = s->second - margin;
            float u = val;
            if (right < val)
                u = right;
            else if (left > val)
                u = left;
            if (fabs(u) < fabs(res))
                res = u;
            foundGood = 2;
        }
    }
    if (foundGood > 1)
        return res;
    else if (foundGood > 0)
        return sres;
    else
        return lres;
}

#endif

inline
Zones::Exclusion  Zones::Exclusion::split_at(float p) {
    Exclusion r(*this);
    r.xm = x = p;
    return r;
}

inline
void Zones::Exclusion::left_trim(float p) {
    x = p;
}

inline
Zones::Exclusion & Zones::Exclusion::operator += (Exclusion const & rhs) {
    c += rhs.c; sm += rhs.sm; smx += rhs.smx; open = false;
    return *this;
}

inline
uint8 Zones::Exclusion::outcode(float p) const {
    return ((p >= xm) << 1) | (p < x);
}


// hmm how to get the margin weight into here
void Zones::exclude_with_margins(float pos, float len, float origin, int axis) {
    origin = 0;
    if (axis < 2)
        weighted<XY>(pos-_margin_len, _margin_len, 0, 0, _margin_weight, pos-_margin_len - origin, 0, 0, false);
    else
        weighted<SD>(pos-_margin_len, _margin_len, 0, 0, _margin_weight, pos-_margin_len - origin, 0, 0, false);
    remove(pos, pos+len);
    if (axis < 2)
        weighted<XY>(pos+len, _margin_len, 0, 0, _margin_weight, pos+len+_margin_len - origin, 0, 0, false);
    else
        weighted<SD>(pos+len, _margin_len, 0, 0, _margin_weight, pos+len+_margin_len - origin, 0, 0, false);
}


void Zones::insert(Exclusion e)
{
#if !defined GRAPHITE2_NTRACING
    addDebug(&e);
#endif
    e.x = std::max(e.x, _pos);
    e.xm = std::min(e.xm, _posm);
    if (e.x >= e.xm) return;

    for (iterator i = _exclusions.begin(), ie = _exclusions.end(); i != ie && e.x < e.xm; ++i)
    {
        const uint8 oca = e.outcode(i->x),
                    ocb = e.outcode(i->xm);
        if ((oca & ocb) != 0) continue;

        switch (oca ^ ocb)  // What kind of overlap?
        {
        case 0:     // e completely covers i
            // split e at i.x into e1,e2
            // split e2 at i.mx into e2,e3
            // drop e1 ,i+e2, e=e3
            *i += e;
            e.left_trim(i->xm);
            break;
        case 1:     // e overlaps on the rhs of i
            // split i at e->x into i1,i2
            // split e at i.mx into e1,e2
            // trim i1, insert i2+e1, e=e2
            if (i->x != e.x && i->xm != e.x)   { i = _exclusions.insert(i,i->split_at(e.x)); ++i; }
            *i += e;
            e.left_trim(i->xm);
            break;
        case 2:     // e overlaps on the lhs of i
            // split e at i->x into e1,e2
            // split i at e.mx into i1,i2
            // drop e1, insert e2+i1, trim i2
            if (e.xm != i->xm) i = _exclusions.insert(i,i->split_at(e.xm));
            *i += e;
            return;
        case 3:     // i completely covers e
            // split i at e.x into i1,i2
            // split i2 at e.mx into i2,i3
            // insert i1, insert e+i2
            if (e.xm != i->xm) i = _exclusions.insert(i,i->split_at(e.xm));
            i = _exclusions.insert(i, i->split_at(e.x));
            *++i += e;
            return;
        }

        ie = _exclusions.end();
    }
}


void Zones::remove(float x, float xm)
{
#if !defined GRAPHITE2_NTRACING
    removeDebug(x, xm);
#endif
    x = std::max(x, _pos);
    xm = std::min(xm, _posm);
    if (x >= xm) return;

    for (iterator i = _exclusions.begin(), ie = _exclusions.end(); i != ie; ++i)
    {
        const uint8 oca = i->outcode(x),
                    ocb = i->outcode(xm);
        if ((oca & ocb) != 0)   continue;

        switch (oca ^ ocb)  // What kind of overlap?
        {
        case 0:     // i completely covers e
            if (i->x != x)  { i = _exclusions.insert(i,i->split_at(x)); ++i; }
            // no break
        case 1:     // i overlaps on the rhs of e
            i->left_trim(xm);
            return;
        case 2:     // i overlaps on the lhs of e
            i->xm = x;
            if (i->x != i->xm) break;
            // no break
        case 3:     // e completely covers i
            i = _exclusions.erase(i);
            --i;
            break;
        }

        ie = _exclusions.end();
    }
}


Zones::const_iterator Zones::find_exclusion_under(float x) const
{
    int l = 0, h = _exclusions.size();

    while (l < h)
    {
        int const p = (l+h) >> 1;
        switch (_exclusions[p].outcode(x))
        {
        case 0 : return _exclusions.begin()+p;
        case 1 : h = p; break;
        case 2 : 
        case 3 : l = p+1; break;
        }
    }

    return _exclusions.begin()+l;
}


float Zones::closest(float origin, float & cost) const
{
    float best_c = std::numeric_limits<float>::max(),
          best_x = 0;

    const const_iterator start = find_exclusion_under(origin);

    // Forward scan looking for lowest cost
    for (const_iterator i = start, ie = _exclusions.end(); i != ie; ++i)
        if (i->track_cost(best_c, best_x, origin)) break;

    // Backward scan looking for lowest cost
    //  We start from the exclusion to the immediate left of start since we've
    //  already tested start with the right most scan above.
    for (const_iterator i = start-1, ie = _exclusions.begin()-1; i != ie; --i)
        if (i->track_cost(best_c, best_x, origin)) break;

    cost = (best_c == std::numeric_limits<float>::max() ? -1 : best_c);
    return best_x;
}


// Cost and test position functions

bool Zones::Exclusion::track_cost(float & best_cost, float & best_pos, float origin) const {
    const float p = test_position(origin),
                localc = cost(p - origin);
    if (open && localc > best_cost) return true;

    if (localc < best_cost)
    {
        best_cost = localc;
        best_pos = p;
    }
    return false;
}


float Zones::Exclusion::cost(float p) const {
    return (sm * p - 2 * smx) * p + c;
}


float Zones::Exclusion::test_position(float origin) const {
    if (sm < 0)
    {
        // sigh, test both ends and perhaps the middle too!
        float res = x;
        float cl = cost(x);
        if (x < origin && xm > origin)
        {
            float co = cost(origin);
            if (co < cl)
            {
                cl = co;
                res = origin;
            }
        }
        float cr = cost(xm);
        return cl > cr ? xm : res;
    }
    else
    {
        float zerox = smx / sm + origin;
        if (zerox < x) return x;
        else if (zerox > xm) return xm;
        else return zerox;
    }
}


#if !defined GRAPHITE2_NTRACING

void Zones::jsonDbgOut(Segment *seg) const {

    if (_dbg)
    {
        for (Zones::idebugs s = dbgs_begin(), e = dbgs_end(); s != e; ++s)
        {
            *_dbg << json::flat << json::array
                << objectid(dslot(seg, (Slot *)(s->_env[0])))
                << reinterpret_cast<ptrdiff_t>(s->_env[1]);
            if (s->_isdel)
                *_dbg << "remove" << Position(s->_excl.x, s->_excl.xm);
            else
                *_dbg << "exclude" << json::flat << json::array
                    << s->_excl.x << s->_excl.xm 
                    << s->_excl.sm << s->_excl.smx << s->_excl.c
                    << json::close;
            *_dbg << json::close;
        }
    }
}

#endif

