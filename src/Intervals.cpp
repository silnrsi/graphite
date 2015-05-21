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
#include <cmath>
#include <limits>

#include "inc/Intervals.h"

using namespace graphite2;


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

// Overlap b on right hand edge of *this.
//   *this is updated to end at the start of b.
//   b is truncated to start at the end *this.
//   The return value is the overlapped zone where parameters are merged.
zones::exclusion zones::exclusion::overlap_by(exclusion & b)
{
    exclusion r(b.x, xm, c+b.c, sm+b.sm, smx+b.smx);
    xm = b.x;
    b.x = r.xm;

    return r;
}

// Cover *this with o
// Update *this with merged parameters.
// o is truncated to start at end of *this.
// Return region over beyond the lhs of *this.
zones::exclusion zones::exclusion::covered_by(exclusion & o)
{
    exclusion r = o;
    r.xm = x;
    o.x = xm;
    c  += o.c;
    sm += o.sm;
    smx += o.smx;

    return r;
}


inline
uint8 zones::exclusion::outcode(float p) const {
    return ((xm < p) << 1) | (p < x);
}

inline
bool zones::exclusion::null_zone() const {
    return c == std::numeric_limits<float>::infinity();
}

inline
bool zones::exclusion::open_zone() const {
    return c == 1 && sm == 0.0f;
}

void zones::exclude(float pos, float len) {
    insert(exclusion(pos, pos+len, std::numeric_limits<float>::infinity(), 0, 0));
}


// hmm how to get the margin weight into here
void zones::exclude_with_margins(float pos, float len) {
    insert(exclusion(pos-_margin_len, pos, 0, _margin_weight, pos-_margin_len));
    insert(exclusion(pos, pos+len, std::numeric_limits<float>::infinity(), 0, 0));
    insert(exclusion(pos+len, pos+len+_margin_len, 0, _margin_weight, pos+len+_margin_len));
}


void zones::insert_tripple(exclusion & l, exclusion & m, exclusion & r)
{

}

void zones::insert(exclusion e)
{
    e.x = std::max(e.x, _pos);
    e.xm = std::min(e.xm, _pos+_len);
    if (e.x >= e.xm) return;

    for (eiter_t i = _exclusions.begin(), ie = _exclusions.end(); i != ie && e.x < e.xm; ++i)
    {
        const uint8 oca = e.outcode(i->x),
                    ocb = e.outcode(i->xm-0.5);
        if ((oca & ocb) == 0)     // We have an overlap here
        {
            switch (oca ^ ocb)  // What kind of overlap?
            {
            case 0:     // e completely covers i
                // split e at i.x into e1,e2
                // split e2 at i.mx into e2,e3
                // i+e2
                if (i->x == e.x)
                    i->covered_by(e);
                else
                {
                    i = _exclusions.insert(i, i->covered_by(e));
                    ++i;
                }
                break;
            case 1:     // e overlaps on the rhs of i
            {
                // split i at e->x into i1,i2
                // split e at i.mx into e1,e2
                // insert i1, i2+e1, insert e2
                exclusion &l = *i;
                if (i->x == e.x)
                    l = l.overlap_by(e);
                else
                    i = _exclusions.insert(++i, l.overlap_by(e));
                break;
            }
            case 2:     // e overlaps on the lhs of i
                // split e at i->x into e1,e2
                // split i at e.mx into i1,i2
                // insert e1, e2+i1, insert i2
                i = _exclusions.insert(i, e.overlap_by(*i));
                i = _exclusions.insert(i, e);
                return;
            case 3:     // i completely covers e
                // split i at e.x into i1,i2
                // split i2 at e.mx into i2,i3
                // e+i2
                if (i->x == e.x)
                {
                    e.covered_by(*i);
                    i = _exclusions.insert(i, e);
                }
                else
                {
                    i = _exclusions.insert(i, e.covered_by(*i));
                    i = _exclusions.insert(++i, e);
                }
                return;
            }

            ie = _exclusions.end();
        }
        else if ((oca & ocb) == 2) // e doesn't overlap i but is completely to it's left
        {
            _exclusions.insert(i, e);
            return;
        }
    }

    // If there is any exclusion remaining just add it to the end
    if (e.x != e.xm)
        _exclusions.push_back(e);
}


zones::const_eiter_t zones::find_exclusion(float x) const
{
#if 1
    // Binary search
    if (_exclusions.size() > 0)
    {
        int pivot = _exclusions.size()/2,
            width = _exclusions.size()/4;

        do
        {
            const exclusion & e = _exclusions[pivot];

            switch (e.outcode(x))
            {
            case 0 : return _exclusions.begin()+pivot;
            case 1 : pivot -= width; break;
            case 2 : pivot += width; break;
            }

            ++width /= 2;
        } while (width > 1);
    }

#else
    // Simple linear scan in case binary search is buggy
    for (const_eiter_t i = _exclusions.begin(), end = _exclusions.end(); i != end; ++i)
        if (i->outcode(x) == 0) return i;
#endif
    return _exclusions.end();
}


float zones::closest(float origin, float width, float & cost) const
{
    float best_c = std::numeric_limits<float>::max(),
          best_x = 0;

    const const_eiter_t start = find_exclusion(origin);

    // Forward scan looking for lowest cost
    for (const_eiter_t i = start, ie = _exclusions.end(); i != ie; ++i)
    {
        if (i->null_zone()) continue;
        if (i->track_cost(best_c, best_x)) break;
    }

    // Backward scan looking for lowest cost
    //  We start from the exclusion to the immediate left of start since we've
    //  already tested start with the right most scan above.
    for (const_eiter_t i = start-1, ie = _exclusions.begin()-1; i != ie; --i)
    {
        if (i->null_zone()) continue;
        if (i->track_cost(best_c, best_x)) break;
    }

    cost = best_c;
    return best_x;
}


// Cost and test position functions these are the only methods that *need* to
// be specialised based on template parameter.

// For cartesian
inline
bool zones::exclusion::track_cost(float & best_cost, float & best_pos) const {
    const float p = test_position(),
                localc = cost(p);
    if (open_zone() && localc > best_cost) return true;

    if (localc < best_cost)
    {
        best_cost = localc;
        best_pos = p;
    }
    return false;
}

float zones::exclusion::test_position() const {
    float d2c = sm;
    if (d2c < 0)
    {
        // sigh, test both ends)
        float cl = cost(x);
        float cr = cost(xm);
        return cl > cr ? xm : x;
    }
    else
    {
        float zerox = smx/sm;
        if (zerox < x) return x;
        else if (zerox > xm) return xm;
        else return zerox;
    }
}

inline
float zones::exclusion::cost(float p) const {
    return (smx * p + sm) * p + c;
}

