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
#include "inc/Main.h"

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

    float len() const { return _len; }
    void len(float l) { _len = l; }
    void clear() { _v.clear(); }
    void add(float min, float max) { add(tpair(min, max)); }
    void add(tpair interval);
    void remove(float min, float max) { remove(tpair(min, max)); }
    void remove(tpair interval);
    float findBestWithMarginAndLimits(float val, float margin, float minMargin, int &isGood);
    size_t size() const { return _v.size(); }

// private:
    ivtpair begin() { return _v.begin(); }
    ivtpair end() { return _v.end(); }

//private:
public: // debugging
    void append(tpair interval) { _v.push_back(interval); }
private:
    // Ranges of movements in a specific direction; a vector is need to represent disjoint ranges.
    float _len;
    vtpair _v;
};


class zones_base
{
    struct exclusion
    {
        float   x,  // x position
                xm, // xmax position
                c,  // constant
                sm, // sum(Mi)
                smx, // sum(MiXi)
                smx2; // sum(MiXi^2)

        exclusion(float x, float w, float c, float smi, float smxi, float smxi2);
        exclusion(float x, float w, float c, float mi, float xi);
        void operator += (const exclusion & rhs);
        uint8 outcode(float p) const;

        exclusion   overlap_by(exclusion & rhs);
        exclusion   covered_by(exclusion & over);

        bool        null_zone() const;
        bool        open_zone() const;
    };

    typedef Vector<exclusion>                    exclusions;
    typedef typename exclusions::iterator        eiter_t;
    typedef typename exclusions::const_iterator  const_eiter_t;

    exclusions  _exclusions;
    float       _margin_len,
                _margin_weight,
                _pos,
                _len;

    friend class exclusion;

public:
    zones_base();

    void initialise(float pos, float len, float margin_len, float margin_weight);

    void exclude(float pos, float len);
    void weighted(float pos, float len, float weight);

private:
    void    insert(exclusion e);
    const_eiter_t find_exclusion(float x) const;
};

enum zones_t {XY, SD};

template<zones_t O>
class zones : public zones_base
{
    struct exclusion : public zones_base::exclusion
    {
        float test_position() const;
        float cost(float x) const;

        bool track_cost(float & cost, float & x) const;
    };

    typedef Vector<exclusion> exclusions;
    exclusions & _exclusions;

    typedef typename exclusions::const_iterator  const_eiter_t;

public:
    zones() : zones_base(), _exclusions(reinterpret_cast<Vector<exclusion>&>(zones_base::_exclusions)) {}

    float closest( float origin, float width, float a, float &cost) const;
};


inline
zones_base::zones_base()
: _margin_len(0), _margin_weight(0),
  _pos(0), _len(0)
{
    _exclusions.reserve(8);
}

inline
void zones_base::initialise(float pos, float len, float margin_len, float margin_weight) {
    _margin_len = margin_len;
    _margin_weight = margin_weight;
    _exclusions.clear();
    weighted(pos, len, 1);
}

inline
void zones_base::weighted(float pos, float len, float weight) {
    insert(exclusion(pos, len, weight, 0));
}

inline
zones_base::exclusion::exclusion(float x, float w, float c, float smi, float smxi, float smxi2)
: x(x_), xm(x+w_), c(c_), sm(smi), smx(smxi), smx2(smxi2)
{}

inline
zones_base::exclusion::exclusion(float x_, float w_, float c_, float mi, float xi)
: x(x_), xm(x+w_), c(c_), sm(mi), smx(mi * xi), smx2(mi * xi * xi)
{}

} // end of namespace graphite2
