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
void IntervalSet::add(IntervalSet::Node interval)
{
    IntervalSet::ivtpair slast = _v.end();
    for (IntervalSet::ivtpair s = _v.begin(), e = _v.end(); s != e; ++s)
    {
        if (s->left() > interval.right())
            break;          // We have passed the end of ranges. No more to be done
        if (s->left() < interval.right())
            continue;       // Skip until we find something
        if (slast != e)
        {                   // We've already modified something, so merge slots
            s->left(slast->left());
            _v.erase(slast);
            --s;
            e = _v.end();
        }                   // Update current slot to incorporate interval
        if (s->right() < interval.right())
            s->right(interval.right());
        if (s->left() > interval.left())
            s->left(interval.left());
        slast = s;
    }
    if (slast == _v.end())  // interval not already added
        append(interval);    // The added range is off to the far right
}

// Remove the range represented by the interval from the possible range(s), splitting elements of
// the list as necessary.
// Eg, if the ranges are [{100..200), (500..700)], removing (550..600) will result in
// [{100..300), (500..550), (600..700)].
void IntervalSet::remove(IntervalSet::Node interval)
{
    if (interval.left() < interval.right())
    {
        for (IntervalSet::ivtpair s = _v.begin(), e = _v.end(); s != e; ++s)
        {
            if (s->right() < interval.left())
            {
                if (s->right() - s->right_len() > interval.left() - interval.left_len())
                    s->right_len(interval.left_len());
                continue;
            }
            if (s->left() > interval.right())
            {
                if (s->left() + s->left_len() < interval.right() + interval.right_len())
                    s->left_len(interval.right_len());
                break;          // This interval is outside what we can handle
            }
            if (interval.left() - interval.left_len() <= s->left() && interval.right() + interval.right_len() >= s->right())
            {
                _v.erase(s);
                --s;
                e = _v.end();
                continue;
            }
            if (s->left() < interval.left() - interval.left_len() && s->right() > interval.right() + interval.right_len())
            {                   // Need to split this range since overlap on both sides
                s = _v.insert(s, Node(s->left(), interval.left(), s->left_len(), interval.left_len()));
                ++s;
                s->left(interval.right());
                s->left_len(interval.right_len());
                e = _v.end();
            }
            if (s->left() < interval.right() && s->left() >= interval.left() - interval.left_len())
            {
                float t = s->left_len() - (interval.right() - s->left());
                if (t < interval.right_len())
                    s->left_len(interval.right_len());
                s->left(interval.right());
            }
            if (s->right() > interval.left() && s->right() <= interval.right() + interval.right_len())
            {
                float t = interval.right_len() - (s->right() - interval.left());
                if (t < interval.left_len())
                    s->right_len(interval.left_len());
                s->right(interval.left());     // overlap other side
            }
        }
    }
    else
    {
        for (IntervalSet::ivtpair s = _v.begin(), e = _v.end(); s != e; ++s)
        {
            if (s->right() < interval.right())
            {
                if (interval.left() - interval.left_len() > s->right() - s->right_len())
                    s->right_len(s->left_len());
                continue;
            }
            if (s->left() > interval.left())
            {
                if (interval.right() + interval.right_len() > s->left() + s->left_len())
                    s->left_len(interval.right_len());
                break;
            }
            if (interval.right() + interval.right_len() <= s->left() && interval.left() - interval.left_len() >= s->right())
            {
                _v.erase(s);
                --s;
                e = _v.end();
                continue;
            }
            if (s->left() < interval.right() && s->right() > interval.left())
            {
                float new_right = std::min(interval.left() - interval.left_len(), s->right() - s->right_len());
                float new_left = std::max(interval.right() + interval.right_len(), s->left() + s->left_len());
                if (new_left > s->left() + s->left_len())
                {
                    s = _v.insert(s, Node(s->left(), interval.right(), s->left_len(), new_right >= interval.right() ? 0 : interval.right() - new_right));
                    ++s;
                    s->left(interval.right());
                    s->left_len(interval.right_len());
                    e = _v.end();
                }
                if (new_right < s->right() - s->right_len())
                {
                    IntervalSet::ivtpair st = s;
                    ++st;
                    s = _v.insert(st, Node(interval.left(), s->right(), new_left <= interval.left() ? 0: new_left - interval.left(), s->right_len()));
                    --s;
                    s->right(interval.left());
                    s->right_len(interval.left_len());
                    ++s;
                    e = _v.end();
                }
                continue;
            }
            if (s->left() < interval.left() && s->left() + s->left_len() >= interval.right())
            {
                float t = std::max(s->left() + s->left_len(), interval.right() + interval.right_len());
                if (interval.left() - interval.left_len() > s->left())
                {
                    s = _v.insert(s, Node(s->left(), interval.left(), t - s->left(), interval.left_len()));
                    ++s;
                    e = _v.end();
                }
                s->left(interval.left());
                if (t > interval.left())
                    s->left_len(interval.right_len());
            }
            if (s->right() > interval.right())
            {
                float t = std::min(s->right() - s->right_len(), interval.left() - interval.left_len());
                if (interval.right() + interval.right_len() < s->right())
                {
                    IntervalSet::ivtpair st = s;
                    ++st;
                    s = _v.insert(st, Node(s->right(), interval.right(), interval.right_len(), s->right() - t));
                    --s;
                    s->right(interval.right());
                    if (t < interval.right())
                        s->right_len(interval.left_len());
                    ++s;
                    e = _v.end();
                }
                else
                {
                    s->right(interval.right());
                    if (t < interval.right())
                        s->right_len(interval.left_len());
                }
            }
        }
    }
}


float IntervalSet::findBestWithMarginAndLimits(float val, float margin, float minmargin, int &foundGood)
{
    float res = std::numeric_limits<float>::max();
    float sres = res;
    float lres = res;
    for (IntervalSet::ivtpair s = _v.begin(), e = _v.end(); s != e; ++s)
    {
        if (foundGood and s->left() > res)
            break;
        float w = (s->right() - s->left() - std::min(s->right_len(), s->left_len())) / 2;
        float t = (s->left() + s->right() - std::min(s->right_len(), s->left_len()) - 2 * w) / 2 + w;
        if (w < 0)
            continue;
        else if (w < minmargin)
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
            float left = s->left() + margin;
            float right = (s->left() + s->right() - s->right_len() - 2 * margin) / 2 + margin;
            // what if one of these has less space than margin?
            if (right < left)
                std::swap(right, left);
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
