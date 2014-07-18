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
#include "inc/Position.h"

using namespace graphite2;

bool Rect::hitTest(Position &offset, Rect &other, Position &othero, float margin)
{
    if (bl.x + offset.x > other.tr.x + othero.x + margin) return false;
    if (tr.x + offset.x + margin < other.bl.x + othero.x) return false;
    if (bl.y + offset.y > other.tr.y + othero.y + margin) return false;
    if (tr.y + offset.y + margin < other.bl.y + othero.y) return false;
    return true;
}

Position Rect::overlap(Position &offset, Rect &other, Position &othero)
{
    float ax = (bl.x + offset.x) - (other.tr.x + othero.x);
    float ay = (bl.y + offset.y) - (other.tr.y + othero.y);
    float bx = (other.bl.x + othero.x) - (tr.x + offset.x);
    float by = (other.bl.y + othero.y) - (tr.y + offset.y);
    return Position((ax > bx ? ax : bx), (ay > by ? ay : by));
}

    
