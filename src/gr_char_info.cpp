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
    If not, write to the Free Software Foundation, Inc., 59 Temple Place, 
    Suite 330, Boston, MA 02111-1307, USA or visit their web page on the 
    internet at http://www.fsf.org/licenses/lgpl.html.
*/
#include <cassert>
#include "graphite2/Segment.h"
#include "CharInfo.h"

extern "C"
{

unsigned int gr_cinfo_unicode_char(const gr_char_info* p/*not NULL*/)
{
    assert(p);
    return p->unicodeChar();
}


int gr_cinfo_break_weight(const gr_char_info* p/*not NULL*/)
{
    assert(p);
    return p->breakWeight();
}

} // extern "C"