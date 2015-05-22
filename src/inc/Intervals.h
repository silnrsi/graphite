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

#include "inc/Main.h"
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

//private
public: // debugging
    void append(tpair interval) { _v.push_back(interval); }
private:
    // Ranges of movements in a specific direction; a vector is need to represent disjoint ranges.
    float _len;
    vtpair _v;
};



enum zones_t {SD, XY};

class Zones
{
protected:
    struct Exclusion
    {
        float   x,  // x position
                xm, // xmax position
                c,  // constant + sum(MiXi^2)
                sm, // sum(Mi)
                smx; // sum(MiXi)

        Exclusion(float x, float w, float c, float smi, float smxi);
        void operator += (const Exclusion & rhs);
        uint8 outcode(float p) const;

        Exclusion   overlap_by(Exclusion & rhs);
        Exclusion   covered_by(Exclusion & over);

        bool        null_zone() const;
        bool        open_zone() const;

        bool track_cost(float & cost, float & x) const;

    private:
        float test_position() const;
        float cost(float x) const;
     };
 

    void insert(Exclusion e);

private:
    typedef Vector<Exclusion>                    exclusions;
    typedef /*typename*/ exclusions::iterator    eiter_t;  // SC: typename does not compile in VisualStudio

    exclusions  _exclusions;
    float       _margin_len,
                _margin_weight,
                _pos,
                _len;

    friend class Exclusion;

public:
    typedef /*typename*/ exclusions::const_iterator  const_eiter_t;   // SC: typename does not compile in VisualStudio

    Zones();
    template<zones_t O>
    void initialise(float pos, float len, float margin_len, float margin_weight, float shift, float oshift, float a);

    void exclude(float pos, float len);
    void exclude_with_margins(float pos, float len);

    template<zones_t O>
    void weighted(float pos, float len, float f, float shift, float oshift, float a, float mi, float xi, float c);

    float closest( float origin, float width, float &cost) const;

    const_eiter_t begin() const { return _exclusions.begin(); }
    const_eiter_t end() const { return _exclusions.end(); }
    

private:
    const_eiter_t find_exclusion(float x) const;
    void insert_triple(Exclusion & l, Exclusion & m, Exclusion & r);

};


inline
Zones::Zones()
: _margin_len(0), _margin_weight(0),
  _pos(0), _len(0)
{
    _exclusions.reserve(8);
}

inline
Zones::Exclusion::Exclusion(float x_, float xm_, float smi, float smxi, float c_)
: x(x_), xm(xm_), c(c_), sm(smi), smx(smxi)
{ }

template<zones_t O>
inline
void Zones::initialise(float pos, float len, float margin_len, float margin_weight, float shift, float oshift, float a)
{
    _margin_len = margin_len;
    _margin_weight = margin_weight;
    _pos = pos;
    _len = len;
    _exclusions.clear();
    weighted<O>(pos, len, 1, shift, oshift, a, 0, 0, 0);
}

template<>
inline
void Zones::weighted<XY>(float pos, float len, float f, float shift, GR_MAYBE_UNUSED float oshift,
			float a, float m, float xi, float c){
    insert(Exclusion(pos, pos+len,
		m + f,
		m * xi + f * shift,
		m * xi * xi + f * shift * shift + c + f * a));
}

template<>
inline
void Zones::weighted<SD>(float pos, float len, float f, float shift, float oshift,
			float a, float m, float xi, float c)
{
    insert(Exclusion(pos, pos+len,
		m + f,
		m * (xi + a) + f * (shift + oshift),
		m * xi * xi + f * (shift + oshift) * (shift + oshift) + c + f * a));
}

} // end of namespace graphite2
