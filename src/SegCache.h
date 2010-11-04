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

#include <graphiteng/GrSegment.h>
#include "Main.h"
#include "SlotImp.h"
#include "SegCacheEntry.h"

namespace org { namespace sil { namespace graphite { namespace v2 {

class SegCache;
class SegCacheEntry;

/**
 * SegPrefixEntry stores lists of word/syllable segments
 * with one list for each word length. The prefix size should be chosen so that
 * these list sizes stay small since they will be searched iteratively.
 */
class SegCachePrefixEntry
{
public:
    SegCachePrefixEntry() { memset(this, 0, sizeof(SegCachePrefixEntry)); }
    friend class SegCache;
    CLASS_NEW_DELETE
private:
    /** m_entries is a null terminated list of entries */
    uint16 m_entryCounts[eMaxCachedSeg];
    SegCacheEntry * m_entries[eMaxCachedSeg];
};


class SegCache
{
public:
    SegCache(const GrFace * face, size_t maxSegments, uint32 flags);
    ~SegCache();

    SegCacheEntry * find(const Slot * firstSlot, size_t length) const;
    SegCacheEntry * cache(const Slot * firstUnprocessedSlot, size_t length, GrSegment * seg);

    uint16 space() const { return m_spaceGid; }
    uint16 maxCmapGlyph() const { return m_maxCmapGid; }

    CLASS_NEW_DELETE
private:
    uint16 m_spaceGid;
    uint16 m_maxCmapGid;
    uint16 m_prefixLength;
    uint16 m_maxCachedSegLength;
    long long m_segmentCount;
    long long m_maxSegmentCount;
    mutable long long m_totalAccessCount;
    void ** m_prefixes;
};

inline SegCacheEntry * SegCache::find(const Slot * firstSlot, size_t length) const
{
    uint16 pos = 0;
    const Slot * slot = firstSlot;
    if (!length) return NULL;
    void ** pEntry = (void **) m_prefixes[firstSlot->gid()];
    while (++pos < m_prefixLength)
    {
        if (!pEntry) return NULL;
        if (slot) slot = slot->next();
        assert(slot || pos >= length);
        pEntry = (void **)pEntry[(pos < length)? slot->gid() : 0];
    }
    if (!pEntry) return NULL;
    SegCachePrefixEntry * prefixEntry = reinterpret_cast<SegCachePrefixEntry*>(pEntry);
    const Slot * lastPrefixSlot = slot;
    
    for (uint16 i = 0; i < prefixEntry->m_entryCounts[length]; i++)
    {
        bool equal = true;
        slot = lastPrefixSlot;
        for (pos = m_prefixLength; pos < length && equal; pos++)
        {
            slot = slot->next();
            equal = (slot->gid() == prefixEntry->m_entries[length][i].m_unicode[pos]);
        }
        if (equal)
        {
            ++m_totalAccessCount;
            prefixEntry->m_entries[length][i].accessed(m_totalAccessCount);
            return prefixEntry->m_entries[length] + i;
        }
    }
    return NULL;
}
    
}}}} // end namespace

