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
#include "GrFaceImp.h"

namespace org { namespace sil { namespace graphite { namespace v2 {

SegCacheStore::SegCacheStore(const GrFace *face, unsigned int numSilf, size_t maxSegments, uint32 flags)
 : m_caches(new SilfSegCache[numSilf]), m_numSilf(numSilf), m_maxCmapGid(0),
   m_maxSegments(maxSegments), m_flags(flags)
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
        void * bmpTable = TtfUtil::FindCmapSubtable(face->getTable(tagCmap, NULL), 3, 1);
        void * supplementaryTable = TtfUtil::FindCmapSubtable(face->getTable(tagCmap, NULL), 3, 10);

        if (bmpTable)
        {
            m_spaceGid = TtfUtil::Cmap31Lookup(bmpTable, 0x20);
            // TODO find out if the Cmap(s) can be parsed to find a m_maxCmapGid < num_glyphs
            // The Pseudo glyphs may mean that it isn't worth the effort
        }
    }
}


}}}}
