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
#include <utility>
#include "inc/List.h"

// An IntervalSet represents the possible movement of a given glyph in a given direction
// (horizontally, vertically, or diagonally).
// A vector is needed to represent disjoint ranges, eg, -300..-150, 20..200, 500..750.
// Each pair represents the min/max of a sub-range.

namespace graphite2 {

class IntervalSet
{
public:
    typedef std::pair<float, float> tpair;
    typedef Vector<tpair> vtpair;
    typedef vtpair::iterator ivtpair;

    IntervalSet locate(float min, float max) { return locate(tpair(min, max)); }
    IntervalSet locate(tpair interval);
    void clear() { _v.clear(); }
    void add(float min, float max) { add(tpair(min, max)); }
    void add(tpair interval);
    void remove(float min, float max) { remove(tpair(min, max)); }
    void remove(tpair interval);
    void remove(IntervalSet &is);
    float findClosestCoverage(float val);
    float findBestWithMarginAndLimits(float val, float margin, float vmin, float vmax, bool &isGood);
    size_t size() const { return _v.size(); }

// private:
    ivtpair begin() { return _v.begin(); }
    ivtpair end() { return _v.end(); }

//private:
public: // debugging
    void append(tpair interval) { _v.push_back(interval); }
private:
    // Ranges of movements in a specific direction; a vector is need to represent disjoint ranges.
    vtpair _v;
};

};

