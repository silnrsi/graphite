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
#include "graphiteng/GrFace.h"

#include "Main.h"
#include "TtfTypes.h"
#include "TtfUtil.h"
#include "SegCache.h"
#include "SegCacheEntry.h"

namespace org { namespace sil { namespace graphite { namespace v2 {

SegCache::SegCache(const GrFace * face, size_t maxSegments, uint32 flags)
    : m_spaceGid(3), m_maxCmapGid(num_glyphs(face)),
    m_prefixLength(ePrefixLength),
    m_maxCachedSegLength(eMaxCachedSeg),
    m_segmentCount(0),
    m_maxSegmentCount(maxSegments),
    m_totalAccessCount(0l),
    m_prefixes(NULL)
{
    void * bmpTable = TtfUtil::FindCmapSubtable(face->getTable(tagCmap, NULL), 3, 1);
    void * supplementaryTable = TtfUtil::FindCmapSubtable(face->getTable(tagCmap, NULL), 3, 10);

    if (bmpTable)
    {
        m_spaceGid = TtfUtil::Cmap31Lookup(bmpTable, 0x20);
        // TODO find out if the Cmap(s) can be parsed to find a m_maxCmapGid < num_glyphs
        // The Pseudo glyphs may mean that it isn't worth the effort
        
    }
    m_prefixes = grzeroalloc<void*>(m_maxCmapGid);
}

SegCache::~SegCache()
{
    // TODO walk tree freeing objects
    free(m_prefixes);
    m_prefixes = NULL;
}


SegCacheEntry* SegCache::cache(const Slot * firstUnprocessedSlot, size_t length, GrSegment * seg)
{
    uint16 pos = 0;
    const Slot * slot = firstUnprocessedSlot;
    if (!length) return NULL;
    void * pEntry = m_prefixes[firstUnprocessedSlot->gid()];
    while (++pos < m_prefixLength)
    {
        if (!pEntry)
        {
            pEntry = grzeroalloc<void*>(m_maxCmapGid);
        }
        if (slot) slot = slot->next();
        assert(slot || pos >= length);
        pEntry = ((void**)pEntry)[(pos < length)? slot->gid() : 0];
    }
    if (!pEntry)
        pEntry = new SegCachePrefixEntry();
    if (!pEntry) return NULL;
    SegCachePrefixEntry * prefixEntry = reinterpret_cast<SegCachePrefixEntry*>(pEntry);
    SegCacheEntry * oldEntries = prefixEntry->m_entries[length];
    size_t listSize = prefixEntry->m_entryCounts[length] + 1;
    prefixEntry->m_entries[length] = gralloc<SegCacheEntry>(listSize);
    if (!prefixEntry->m_entries[length])
    {
        // out of memory
        free(oldEntries);
        prefixEntry->m_entryCounts[length] = 0;
        return NULL;
    }
    else
    {
        prefixEntry->m_entryCounts[length] = listSize;
    }
    if (listSize > 1)
    {
        memcpy(prefixEntry->m_entries[length], oldEntries, sizeof(SegCacheEntry) * (listSize-1));
        free(oldEntries);
    }
    ::new (prefixEntry->m_entries[length] + listSize - 1)
        SegCacheEntry(firstUnprocessedSlot, length, seg, m_totalAccessCount);
    return prefixEntry->m_entries[length] + listSize - 1;
}

}}}} // namespace
