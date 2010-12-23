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

#include <graphite2/Segment.h>
#include "TtfTypes.h"
#include "locale2lcid.h"

namespace graphite2 {

class NameTable
{
public:
    NameTable(const void * data, size_t length, uint16 platfromId=3, uint16 encodingID = 1);
    ~NameTable() {};
    enum {
        eNoFallback = 0,
        eEnUSFallbackOnly = 1,
        eEnOrAnyFallback = 2
    } eNameFallback;
    uint16 setPlatformEncoding(uint16 platfromId=3, uint16 encodingID = 1);
    void * getName(uint16 & languageId, uint16 nameId, gr_encform enc, uint32 & length);
    uint16 getLanguageId(const char * bcp47Locale);

    CLASS_NEW_DELETE
private:
    uint16 m_platformId;
    uint16 m_encodingId;
    uint16 m_languageCount;
    uint16 m_platformOffset; // offset of first NameRecord with for platform 3, encoding 1
    uint16 m_platformLastRecord;
    uint16 m_nameDataLength;
    const TtfUtil::Sfnt::FontNames * m_table;
    const uint8 * m_nameData;
    Locale2Lang m_locale2Lang;
};

} // namespace graphite2
