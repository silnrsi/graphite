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

//#include "inc/FeatureMap.h"
//#include "inc/GlyphsCache.h"
//#include "inc/Silf.h"

#ifndef GRAPHITE2_NFILEFACE

#include <cstdio>
#include <cassert>

#include "graphite2/Font.h"

#include "inc/Main.h"
#include "inc/TtfTypes.h"
#include "inc/TtfUtil.h"

namespace graphite2 {

using TtfUtil::Tag;

// These are the actual tags, as distinct from the consecutive IDs in TtfUtil.h

class TableCacheItem
{
public:
    TableCacheItem(char * theData, size_t theSize) : m_data(theData), m_size(theSize) {}
    TableCacheItem() : m_data(0), m_size(0) {}
    ~TableCacheItem()
    {
        if (m_size) free(m_data);
    }
    void set(char * theData, size_t theSize) { m_data = theData; m_size = theSize; }
    const void * data() const { return m_data; }
    size_t size() const { return m_size; }
private:
    char * m_data;
    size_t m_size;
};



class FileFace
{
    static const void *table_fn(const void* appFaceHandle, unsigned int name, size_t *len);
public:
    FileFace(const char *filename);
    ~FileFace();
//    virtual const void *getTable(unsigned int name, size_t *len) const;
    bool isValid() const { return m_pfile && m_pHeader && m_pTableDir; }

    static const gr_face_ops ops;

    CLASS_NEW_DELETE;
public:     //for local convenience    
    FILE* m_pfile;
    unsigned int m_lfile;
    mutable TableCacheItem m_tables[18];
    TtfUtil::Sfnt::OffsetSubTable* m_pHeader;
    TtfUtil::Sfnt::OffsetSubTable::Entry* m_pTableDir;       //[] number of elements is determined by m_pHeader->num_tables
   
private:        //defensive
    FileFace(const FileFace&);
    FileFace& operator=(const FileFace&);
};

} // namespace graphite2

#endif      //!GRAPHITE2_NFILEFACE
