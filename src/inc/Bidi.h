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
#pragma once

namespace graphite2
{

class BracketPair
{
public:
    BracketPair(uint16 g, Slot *s, uint8 b) : _open(s), _gid(g), _mask(0), _before(b) {};
    uint16 gid() const { return _gid; }
    Slot *open() const { return _open; }
    uint8 mask() const { return _mask; }
    int8 before() const { return _before; }
    void orin(uint8 m) { _mask |= m; }
private:
    Slot  * _open;
    uint16  _gid;
    uint8   _mask;
    int8    _before;
};

class BracketPairStack
{
public:
    BracketPairStack(uint s) : _stack(grzeroalloc<BracketPair>(s)), _tos(_stack - 1), _size(s) {}
    ~BracketPairStack() { free(_stack); }

public:
    BracketPair *scan(uint16 gid);
    void reset(BracketPair *tos) { _tos = tos ? tos : _stack; --_tos; }
    BracketPair *push(uint16 gid, Slot *pos, uint8 before) {
        if (++_tos - _stack < _size)
            ::new (_tos) BracketPair(gid, pos, before);
        return _tos;
    }
    void orin(uint8 mask) { if (_tos >= _stack) _tos->orin(mask); }
    uint size() const { return _size; }

    CLASS_NEW_DELETE

private:

    BracketPair *_stack;
    BracketPair *_tos;
    uint         _size;
};

inline BracketPair *BracketPairStack::scan(uint16 gid)
{
    BracketPair *res = _tos;
    while (res > _stack)
    {
        if (res->gid() == gid)
            return res;
        --res;
    }
    return 0;
}

}
