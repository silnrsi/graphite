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
#include <cstring>
//#include "graphite2/Segment.h"
//#include "inc/CmapCache.h"
//#include "inc/Endian.h"
#include "inc/FileFace.h"
//#include "inc/GlyphFace.h"
//#include "inc/SegCacheStore.h"
//#include "inc/Segment.h"
//#include "inc/NameTable.h"


using namespace graphite2;

#ifndef GRAPHITE2_NFILEFACE

FileFace::FileFace(const char *filename) :
    m_pHeader(NULL),
    m_pTableDir(NULL)
{
    if (!(m_pfile = fopen(filename, "rb"))) return;
    if (fseek(m_pfile, 0, SEEK_END)) return;
    m_lfile = ftell(m_pfile);
    if (fseek(m_pfile, 0, SEEK_SET)) return;
    size_t lOffset, lSize;
    if (!TtfUtil::GetHeaderInfo(lOffset, lSize)) return;
    m_pHeader = (TtfUtil::Sfnt::OffsetSubTable*)gralloc<char>(lSize);
    if (fseek(m_pfile, lOffset, SEEK_SET)) return;
    if (fread(m_pHeader, 1, lSize, m_pfile) != lSize) return;
    if (!TtfUtil::CheckHeader(m_pHeader)) return;
    if (!TtfUtil::GetTableDirInfo(m_pHeader, lOffset, lSize)) return;
    m_pTableDir = (TtfUtil::Sfnt::OffsetSubTable::Entry*)gralloc<char>(lSize);
    if (fseek(m_pfile, lOffset, SEEK_SET)) return;
    if (fread(m_pTableDir, 1, lSize, m_pfile) != lSize) return;
}

FileFace::~FileFace()
{
    free(m_pTableDir);
    free(m_pHeader);
    if (m_pfile)
        fclose(m_pfile);
    m_pTableDir = NULL;
    m_pfile = NULL;
    m_pHeader = NULL;
}


const void *FileFace::table_fn(const void* appFaceHandle, unsigned int name, size_t *len)
{
    const FileFace* ttfFaceHandle = (const FileFace*)appFaceHandle;
    TableCacheItem * tci = ttfFaceHandle->m_tables;

    switch (name)
    {
		case Tag::Feat:	tci += 0; break;
		case Tag::Glat:	tci += 1; break;
		case Tag::Gloc:	tci += 2; break;
		case Tag::OS_2:	tci += 3; break;
		case Tag::Sile:	tci += 4; break;
		case Tag::Silf:	tci += 5; break;
		case Tag::Sill:	tci += 6; break;
    	case Tag::cmap:	tci += 7; break;
    	case Tag::glyf:	tci += 8; break;
    	case Tag::hdmx:	tci += 9; break;
    	case Tag::head:	tci += 10; break;
    	case Tag::hhea:	tci += 11; break;
    	case Tag::hmtx:	tci += 12; break;
    	case Tag::kern:	tci += 13; break;
    	case Tag::loca:	tci += 14; break;
    	case Tag::maxp:	tci += 15; break;
    	case Tag::name:	tci += 16; break;
    	case Tag::post:	tci += 17; break;
    	default:					tci = 0; break;
    }

    assert(tci); // don't expect any other table types
    if (!tci) return NULL;
    if (tci->data() == NULL)
    {
        char *tptr;
        size_t tlen, lOffset;
        if (!TtfUtil::GetTableInfo(name, ttfFaceHandle->m_pHeader, ttfFaceHandle->m_pTableDir, lOffset, tlen)) return NULL;
        if (fseek(ttfFaceHandle->m_pfile, lOffset, SEEK_SET)) return NULL;
        if (lOffset + tlen > ttfFaceHandle->m_lfile) return NULL;
        tptr = gralloc<char>(tlen);
        if (fread(tptr, 1, tlen, ttfFaceHandle->m_pfile) != tlen) 
        {
            free(tptr);
            return NULL;
        }
        tci->set(tptr, tlen);
    }
    if (len) *len = tci->size();
    return tci->data();
}

const gr_face_ops FileFace::ops = { &FileFace::table_fn, 0, 0 };

#endif                  //!GRAPHITE2_NFILEFACE
