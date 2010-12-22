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

#include "SegCacheStore.h"
#include "Face.h"


SegCacheStore::SegCacheStore(const Face *face, unsigned int numSilf, size_t maxSegments)
 : m_caches(new SilfSegCache[numSilf]), m_numSilf(numSilf), m_maxSegments(maxSegments),
   m_maxCmapGid(0)
{
    assert(face);
    assert(face->getGlyphFaceCache());
    m_maxCmapGid = face->getGlyphFaceCache()->numGlyphs();
    if (face->getCmapCache())
    {
        m_spaceGid = face->getCmapCache()->lookup(0x20);
        m_zwspGid = face->getCmapCache()->lookup(0x200B);
    }
    else
    {
        size_t cmapSize = 0;
        const void * cmapTable = face->getTable(tagCmap, &cmapSize);
        const void * bmpTable = TtfUtil::FindCmapSubtable(cmapTable, 3, 1, cmapSize);
        //const void * supplementaryTable = TtfUtil::FindCmapSubtable(cmapTable, 3, 10, cmapSize);

        if (bmpTable)
        {
            m_spaceGid = TtfUtil::Cmap31Lookup(bmpTable, 0x20);
            m_zwspGid = TtfUtil::Cmap31Lookup(bmpTable, 0x200B);
            // TODO find out if the Cmap(s) can be parsed to find a m_maxCmapGid < num_glyphs
            // The Pseudo glyphs may mean that it isn't worth the effort
        }
    }
}


