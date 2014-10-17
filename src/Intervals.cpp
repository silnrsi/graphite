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

using namespace graphite2;


IntervalSet IntervalSet::locate(IntervalSet::tpair interval)
{
    IntervalSet res;
    for (IntervalSet::ivtpair s = _v.begin(), e = _v.end(); s != e; s++)
    {
        if (s->second - s->first > (interval.second - interval.first))
            res.append(IntervalSet::tpair(s->first - interval.first, s->second - interval.second));
    }
    return res;
}

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
        if (s->second < interval.first)
            continue;
        if (s->first > interval.second)
            break;          // This interval is outside what we can handle
        if (s->first < interval.first && s->second > interval.second)
        {                   // Need to split this range since overlap on both sides
            s = _v.insert(s, tpair(s->first, interval.first));
            ++s;
            s->first = interval.second;
            e = _v.end();
        }
        if (s->first < interval.second && s->first > interval.first)
            s->first = interval.second;     // overlap one side
        if (s->second > interval.first && s->second < interval.second)
            s->second = interval.first;     // overlap other side
    }
}

void IntervalSet::remove(IntervalSet &is)
{
    IntervalSet::ivtpair t = is.begin(), tend = is.end();
    for (IntervalSet::ivtpair s = _v.begin(), e = _v.end(); s != e; )
    {
        if (s->second < t->first)
        {
            ++s;
            continue;                   // scan to find first overlap
        }
        if (s->first < t->first && s->second > t->second)
        {                               // overlap on both sides
            s = _v.insert(s, tpair(s->first, t->first));
            ++s;
            s->first = t->second;
            e = _v.end();
        }
        if (s->first < t->second && s->first >= t->first)
            s->first = t->second;       // overlap on one side
        if (s->second > t->first && s->second <= t->second)
            s->second = t->first;       // overlap on other side
        if (t->second < s->second)
        {
            if (++t == tend)
                break;
            else
                continue;
        }
        if (s->first >= s->second)
        {
            _v.erase(s);
            e = _v.end();
            --s;
        }
        ++s;
    }
}

// Find a legal interval corresponding to val. Return how much off val is?????????????
float IntervalSet::findClosestCoverage(float val)
{
    float best = -std::numeric_limits<float>::max();
    // Better to use a binary search here
    for (IntervalSet::ivtpair s = _v.begin(), e = _v.end(); s != e; ++s)
    {
        if (s->second < val)
            best = s->second;
        else if (s->first > val)
        {
            if (val - s->first > best)
                best = s->first;
            return best;
        }
        else
            return val;
    }
    return best;
}
