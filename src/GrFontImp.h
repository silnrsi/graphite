/*  GRAPHITENG LICENSING

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
    If not, write to the Free Software Foundation, Inc., 59 Temple Place, 
    Suite 330, Boston, MA 02111-1307, USA or visit their web page on the 
    internet at http://www.fsf.org/licenses/lgpl.html.
*/
#pragma once
#include <cassert>
#include "graphiteng/GrFont.h"
#include "Main.h"
#include "GrFaceImp.h"

namespace org { namespace sil { namespace graphite { namespace v2 {

const float INVALID_ADVANCE = -1e38f;		//because this is in the header it can be optimized out.

class GrFont
{
public:
    GrFont(float ppm, const GrFace *face/*needed for scaling*/);
public:
    virtual ~GrFont();
    float advance(unsigned short glyphid) const {
        if (m_advances[glyphid] == INVALID_ADVANCE)
            m_advances[glyphid] = computeAdvance(glyphid);
        return m_advances[glyphid];
    }
//    Position scale(const Position& p) const { return Position(m_scale * p.x, m_scale * p.y); }
//    float scale(float p) const { return m_scale * p; }
    float scale() const { return m_scale; }
    virtual bool isHinted() const { return false; }

    CLASS_NEW_DELETE
private:
    virtual float computeAdvance(unsigned short glyphid) const { assert(false); return .0f; };
    
protected:
    float m_scale;      // scales from design units to ppm
    float *m_advances;  // One advance per glyph in pixels. Nan if not defined
    
private:			//defensive on m_advances
    GrFont(const GrFont&);
    GrFont& operator=(const GrFont&);
};

class GrSimpleFont : public GrFont      //has no external hints - gets advance information from the face
{
public:
    GrSimpleFont(float ppm/*pixels per em*/, const GrFace *face);
private:
    virtual float computeAdvance(unsigned short glyphid) const;
private:
    const GrFace *m_face;   // GrFace to get the rest of the info from
};

class GrHintedFont : public GrFont
{
public:
    GrHintedFont(float ppm/*pixels per em*/, const void* appFontHandle/*non-NULL*/, advance_fn advance, const GrFace *face/*needed for scaling*/);
    virtual bool isHinted() const { return true; }
private:
    virtual float computeAdvance(unsigned short glyphid) const;

private:
    const void* m_appFontHandle/*non-NULL*/;
    advance_fn m_advance;
};

}}}} // namespace
