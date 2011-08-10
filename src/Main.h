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

Alternatively, the contents of this file may be used under the terms of the
Mozilla Public License (http://mozilla.org/MPL) or the GNU General Public
License, as published by the Free Software Foundation, either version 2
of the License or (at your option) any later version.
*/
#pragma once

#include <cstdlib>
#include "graphite2/Types.h"


namespace graphite2 {

typedef gr_uint8        uint8;
typedef gr_uint8        byte;
typedef gr_uint16       uint16;
typedef gr_uint32       uint32;
typedef gr_int8         int8;
typedef gr_int16        int16;
typedef gr_int32        int32;
typedef size_t          uintptr;

#if !defined WORDS_BIGENDIAN || defined PC_OS
inline uint16 swap16(uint16 x) { return (x << 8) | (x >> 8); }
inline  int16 swap16(int16 x)  { return int16(swap16(uint16(x))); }
inline uint32 swap32(uint32 x) { return (uint32(swap16(uint16(x))) << 16) | swap16(uint16(x >> 16)); }
inline  int32 swap32(int32 x)  { return int16(swap16(uint16(x))); }
#else
#define swap16(x) (x)
#define swap32(x) (x)
#endif

inline uint16 read16(const byte *&x) { 
  const uint16 r = swap16(*reinterpret_cast<const uint16 *&>(x));
  x += sizeof(uint16);
  return r;
}
inline uint16 read16(byte *&x) { return read16(const_cast<const byte * &>(x)); }
inline uint32 read32(const byte *&x) { 
  const uint32 r = swap32(*reinterpret_cast<const uint32 *&>(x));
  x += sizeof(uint32);
  return r;
}
inline uint32 read32(byte *&x) { return read32(const_cast<const byte * &>(x)); }

// typesafe wrapper around malloc for simple types
// use free(pointer) to deallocate
template <typename T> T * gralloc(size_t n)
{
    return reinterpret_cast<T*>(malloc(sizeof(T) * n));
}

template <typename T> T * grzeroalloc(size_t n)
{
    return reinterpret_cast<T*>(calloc(n, sizeof(T)));
}

} // namespace graphite2

#define CLASS_NEW_DELETE \
    void * operator new[](size_t size) {return malloc(size);} \
    void operator delete[] (void * p)throw() { if (p) free(p); } \
    void * operator new(size_t size){ return malloc(size);} \
    void operator delete (void * p) throw() {if (p) free(p);}

#ifdef __GNUC__
#define GR_MAYBE_UNUSED __attribute__((unused))
#else
#define GR_MAYBE_UNUSED
#endif
