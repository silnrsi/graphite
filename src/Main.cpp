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

// local definitions of new, delete
#include <cstdlib>
#include <cassert>
#include <cstdio>
#include "Main.h"

#ifdef __GNUC__

// define a few basic functions to save using libstdc++

extern "C" __attribute__ ((visibility("internal"))) void __cxa_pure_virtual()
{
    assert(false);
}

// declaration in c++/x.y/bits/functexcept.h
namespace std
{
    // it would be nice to remove these eventually
    // currently they are needed by ext/malloc_allocator.h and vector
    void __attribute__ ((visibility("internal"))) __throw_bad_alloc(void) { assert(false); };
    void __attribute__ ((visibility("internal"))) __throw_length_error(const char* c)
    { fprintf(stderr, "Length error %s\n", c); assert(false);}
}

#endif
