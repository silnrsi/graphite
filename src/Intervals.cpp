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
#include "inc/Intervals.h"
#include <limits>
#include <math.h>
#include <algorithm>

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
            e = _v.end();
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
