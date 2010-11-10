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
    ~SegCachePrefixEntry()
    {
        for (size_t j = 0; j < eMaxCachedSeg; j++)
        {
            if (m_entryCounts[j])
            {
                assert(m_entries[j]);
                for (size_t k = 0; k < m_entryCounts[j]; k++)
                {
                    m_entries[j][k].log(j);
                    m_entries[j][k].clear();
                }
                free(m_entries[j]);
            }
        }
    }
    const SegCacheEntry * find(const uint16 * cmapGlyphs, size_t length) const
    {
        if (length <= ePrefixLength)
        {
            assert(m_entryCounts[length] <= 1);
            if (m_entries[length])
                return m_entries[length];
            return NULL;
        }
        for (uint16 i = 0; i < m_entryCounts[length]; i++)
        {
            bool equal = true;
            for (size_t pos = ePrefixLength; pos < length && equal; pos++)
            {
                equal = (cmapGlyphs[pos] == m_entries[length][i].m_unicode[pos]);
            }
            if (equal)
            {
                return m_entries[length] + i;
            }
        }
        return NULL;
    }
    SegCacheEntry * cache(const uint16* cmapGlyphs, size_t length, GrSegment * seg, size_t charOffset, unsigned long long totalAccessCount)
    {
        SegCacheEntry * oldEntries = m_entries[length];
        size_t listSize = m_entryCounts[length] + 1;
        // hack TODO sort the list and use a binary search
        // the problem comes when you get incremental numeric ids in a large doc
        //if (listSize > 8)
        //    return NULL;
        m_entries[length] = gralloc<SegCacheEntry>(listSize);
        if (!m_entries[length])
        {
            // out of memory
            free(oldEntries);
            m_entryCounts[length] = 0;
            return NULL;
        }
        else
        {
            m_entryCounts[length] = listSize;
        }
        if (listSize > 1)
        {
            memcpy(m_entries[length], oldEntries, sizeof(SegCacheEntry) * (listSize-1));
            free(oldEntries);
        }
        ::new (m_entries[length] + listSize - 1)
            SegCacheEntry(cmapGlyphs, length, seg, charOffset, totalAccessCount);
        return m_entries[length] + listSize - 1;
    }
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

    const SegCacheEntry * find(const uint16 * cmapGlyphs, size_t length) const;
    SegCacheEntry * cache(const uint16 * cmapGlyphs, size_t length, GrSegment * seg, size_t charOffset);

    uint16 space() const { return m_spaceGid; }
    uint16 maxCmapGlyph() const { return m_maxCmapGid; }
    long long totalAccessCount() const { return m_totalAccessCount; }
    size_t segmentCount() const { return m_segmentCount; }

    CLASS_NEW_DELETE
private:
    void freeLevel(void ** prefixes, size_t level);

    uint16 m_spaceGid;
    uint16 m_maxCmapGid;
    uint16 m_prefixLength;
    uint16 m_maxCachedSegLength;
    size_t m_segmentCount;
    size_t m_maxSegmentCount;
    mutable unsigned long long m_totalAccessCount;
    mutable unsigned long long m_totalMisses;
    void ** m_prefixes;
};

inline const SegCacheEntry * SegCache::find(const uint16 * cmapGlyphs, size_t length) const
{
    uint16 pos = 0;
    if (!length || length > eMaxCachedSeg) return NULL;
    void ** pEntry = (void **) m_prefixes[cmapGlyphs[0]];
    while (++pos < m_prefixLength)
    {
        if (!pEntry)
        {
            ++m_totalMisses;
            return NULL;
        }
        pEntry = (void **)pEntry[(pos < length)? cmapGlyphs[pos] : 0];
    }
    if (!pEntry)
    {
        ++m_totalMisses;
        return NULL;
    }
    SegCachePrefixEntry * prefixEntry = reinterpret_cast<SegCachePrefixEntry*>(pEntry);
    const SegCacheEntry * entry = prefixEntry->find(cmapGlyphs, length);
    if (entry)
    {
        ++m_totalAccessCount;
        entry->accessed(m_totalAccessCount);
    }
    else
    {
        ++m_totalMisses;
    }   
    return entry;
}
    
}}}} // end namespace

