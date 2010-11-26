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

#include "Main.h"

namespace org { namespace sil { namespace graphite { namespace v2 {

class CharInfo // : ICharInfo
{

public:
    void init(int cid, int gindex) { m_char = cid; m_before = m_after = gindex; }
    void update(int offset) { m_before += offset; m_after += offset; }
    unsigned int unicodeChar() const { return m_char; }
    void feats(int offset) { m_featureid = offset; }
    int fid() const { return m_featureid; }
    int breakWeight() const { return m_break; }
    void breakWeight(int val) { m_break = val; }
    int glyphBefore() const { return m_before; }
    int glyphAfter() const { return m_after; }

    CLASS_NEW_DELETE
private:
    int m_char;     // Unicode character from character stream
    int m_before;   // slot id of glyph that cursor before this char is before
    int m_after;    // slot id of glyph that cursor after this char is after
    uint8 m_featureid;	// index into features list in the segment
    int8 m_break;	// breakweight coming from lb table
};

}}}} // namespace
