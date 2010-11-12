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

#include "graphiteng/Types.h"

namespace org { namespace sil { namespace graphite { namespace v2 {

class CharInfo;

enum {
    /* after break weights */
    eBreakWhitespace = 10,
    eBreakWord = 15,
    eBreakIntra = 20,
    eBreakLetter = 30,
    eBreakClip = 40,
    /* before break weights */
    eBreakBeforeWhitespace = -10,
    eBreakBeforeWord = -15,
    eBreakBeforeIntra = -20,
    eBreakBeforeLetter = -30,
    eBreakBeforeClip = -40
};

extern "C"
{
    GRNG_EXPORT unsigned int cinfo_unicode_char(const CharInfo* p/*not NULL*/);
    GRNG_EXPORT int cinfo_break_weight(const CharInfo* p/*not NULL*/);
}


}}}} // namespace
