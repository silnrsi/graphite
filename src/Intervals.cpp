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

using namespace graphite2;


// If the given interval (the current location/size of a glyph) will fit in a legal range,
// return the minmum and maximum offsets that will make the interval legal.
// We might return several of these if there are several viable ranges.
// Eg, if the ranges are [(200,300), (450, 525)] and interval = [150,200], 
// return [(50,100), (300,325)]
IntervalSet IntervalSet::locate(IntervalSet::tpair interval)
{
    IntervalSet res;
    for (IntervalSet::ivtpair s = _v.begin(), e = _v.end(); s != e; s++)
    {
        if (s->second - s->first >= (interval.second - interval.first))
            res.append(IntervalSet::tpair(s->first - interval.first, s->second - interval.second));
    }
    return res;
}

IntervalSet IntervalSet::locate(IntervalSet &is)
{
    IntervalSet res;
    for (IntervalSet::ivtpair s = is.begin(), e = is.end(); s != e; ++s)
    {
        IntervalSet temp = locate(*s);
        if (s == is.begin())
            res = temp;
        else
            res.intersect(temp);
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
        if (interval.first <= s->first && interval.second >= s->second)
        {
            _v.erase(s);
            --s;
            e = _v.end();
            continue;
        }
        if (s->first < interval.first && s->second > interval.second)
        {                   // Need to split this range since overlap on both sides
            s = _v.insert(s, tpair(s->first, interval.first));
            ++s;
            s->first = interval.second;
            e = _v.end();
        }
        if (s->first < interval.second && s->first >= interval.first)
            s->first = interval.second;     // overlap one side
        if (s->second > interval.first && s->second <= interval.second)
            s->second = interval.first;     // overlap other side
    }
}

// Remove one interval set from another. Akin to removing each of the pairs in one set from
// this, in sequence. It works faster because we know that interval sets are ordered.
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
        if (s->first >= t->first && s->second <= t->second)
        {
            _v.erase(s);
            e = _v.end();
            continue;
        }
        if (s->first < t->first && s->second > t->second)
        {                               // overlap on both sides
            s = _v.insert(s, tpair(s->first, t->first));
            ++s;
            s->first = t->second;
            e = _v.end();
        }
        if (s->first < t->second && s->first >= t->first)
            s->first = t->second;       // overlap beginning of existing range
        if (s->second > t->first && s->second <= t->second)
            s->second = t->first;       // overlap end of existing range
        if (s->first >= s->second)      // remove duplicates
        {
            _v.erase(s);
            e = _v.end();
            --s;
        }
        if (t->second < s->second)
        {
            if (++t == tend)
                break;
            else
                continue;
        }
        ++s;
    }
}

void IntervalSet::intersect(IntervalSet &is)
{
    IntervalSet::ivtpair t = is.begin(), tend = is.end();
    for (IntervalSet::ivtpair s = _v.begin(), e = _v.end(); s != e; )
    {
        if (s->second < t->first)
        {
            _v.erase(s);
            e = _v.end();
            continue;
        }
        if (s->first < t->first)
            s->first = t->first;
        if (t->second < s->second)
        {
            s = _v.insert(s, *s);
            s->second = t->second;
            ++s;
            s->first = t->second;
            e = _v.end();

            if (++t == tend)
            {
                while (s != e)
                {
                    _v.erase(s);
                    e = _v.end();
                }
                break;
            }
            continue;
        }
        ++s;
    }
}

// Find a legal interval corresponding to val. Return the closest legal value.
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

float IntervalSet::findBestWithMarginAndLimits(float val, float margin, float vmin, float vmax, bool &isGood)
{
    float best = std::numeric_limits<float>::max();
    float sbest = -best;
    isGood = false;
    for (IntervalSet::ivtpair s = _v.begin(), e = _v.end(); s != e; ++s)
    {
        if (s->second < val)
        {
            if (s->second - s->first > 2 * margin && s->second - margin > vmin && s->second - margin < vmax)
            {
                isGood = true;
                best = s->second - margin;
            }
            else if (s->first > sbest && (s->first + s->second) / 2 > vmin && (s->first + s->second) / 2 < vmax)
                sbest = (s->first + s->second) / 2;  // midway between the two for maximum margin
        }
        else if (s->first < val)
        {
            if (val > s->first + margin)
            {
                if (val < s->second - margin)
                {
                    isGood = true;
                    return val;
                }
                else if (s->second - s->first > 2 * margin)
                {
                    isGood = true;
                    return s->second - margin;
                }
                else
                    sbest = (s->first + s->second) / 2;
            }
            else if (s->second - s->first > 2 * margin)
            {
                isGood = true;
                return s->first + margin;
            }
            else
                sbest = (s->first + s->second) / 2;
        }
        else if ((s->second - s->first) > 2 * margin)
        {
            if (s->first + margin > vmin && s->first + margin < vmax)
            {
                if (!isGood || s->first + margin < -best)
                {
                    best = s->first + margin;
                    isGood = true;
                    return best;
                }
                else if (fabs(sbest) < (s->first + s->second) / 2)
                    sbest = (s->first + s->second) / 2;
            }
        }
        else if (!isGood && (s->first + s->second) / 2 < -sbest && (s->first + s->second) / 2 > vmin && (s->first + s->second) / 2 < vmax)
        {
            sbest = (s->first + s->second) / 2;
        }
    }
    if (isGood)
        return best;
    else
        return sbest;
}

