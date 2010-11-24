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

#include <cstddef>

namespace org { namespace sil { namespace graphite { namespace v2 {

typedef unsigned char uint8;
typedef uint8    byte;
typedef signed char int8;
typedef unsigned short uint16;
typedef short   int16;
typedef unsigned int    uint32;
typedef int     int32;

enum encform {
  kutf8 = 1/*sizeof(uint8)*/, kutf16 = 2/*sizeof(uint16)*/, kutf32 = 4/*sizeof(uint32)*/
};

#ifdef _MSC_VER
#define GRNG_EXPORT __declspec(dllexport)
#else
#ifdef __GNUC__
#define GRNG_EXPORT __attribute__ ((visibility("default")))
#else
#define GRNG_EXPORT
#endif
#endif


}}}} // namespace

