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
#include "CmapCache.h"

namespace org { namespace sil { namespace graphite { namespace v2 {

SegCache::SegCache(const GrFace * face, const Features & feats, size_t maxSegments, uint32 flags)
    : m_spaceGid(3), m_maxCmapGid(face_num_glyphs(face)),
    m_prefixLength(ePrefixLength),
    m_maxCachedSegLength(eMaxCachedSeg),
    m_segmentCount(0),
    m_maxSegmentCount(maxSegments),
    m_totalAccessCount(0l), m_totalMisses(0l),
    m_features(feats)
{
    if (face->getCmapCache())
    {
        m_spaceGid = face->getCmapCache()->lookup(0x20);
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
    m_prefixes.raw = grzeroalloc<void*>(m_maxCmapGid+2);
    m_prefixes.range[SEG_CACHE_MIN_INDEX] = SEG_CACHE_UNSET_INDEX;
    m_prefixes.range[SEG_CACHE_MAX_INDEX] = SEG_CACHE_UNSET_INDEX;
    assert(sizeof(void*) == sizeof(uintptr_t));
}

void SegCache::freeLevel(SegCachePrefixArray prefixes, size_t level)
{
    for (size_t i = 0; i < m_maxCmapGid; i++)
    {
        if (prefixes.array[i].raw)
        {
            if (level + 1 < ePrefixLength)
                freeLevel(prefixes.array[i], level + 1);
            else
            {
                SegCachePrefixEntry * prefixEntry = prefixes.prefixEntries[i];
                delete prefixEntry;
            }
        }
    }
    free(prefixes.raw);
}

SegCache::~SegCache()
{
#ifndef DISABLE_TRACING
    if (XmlTraceLog::get().active())
    {
        XmlTraceLog::get().openElement(ElementSegCache);
        XmlTraceLog::get().addAttribute(AttrNum, m_segmentCount);
        XmlTraceLog::get().addAttribute(AttrAccessCount, m_totalAccessCount);
        XmlTraceLog::get().addAttribute(AttrMisses, m_totalMisses);
    }
#endif
    freeLevel(m_prefixes, 0);
#ifndef DISABLE_TRACING
    if (XmlTraceLog::get().active())
    {
        XmlTraceLog::get().closeElement(ElementSegCache);
    }
#endif
    m_prefixes.raw = NULL;
}

SegCacheEntry* SegCache::cache(const uint16* cmapGlyphs, size_t length, GrSegment * seg, size_t charOffset)
{
    uint16 pos = 0;
    if (!length) return NULL;
    assert(length < m_maxCachedSegLength);
    SegCachePrefixArray pArray = m_prefixes;
    while (pos + 1 < m_prefixLength)
    {
        uint16 gid = (pos < length)? cmapGlyphs[pos] : 0;
        if (!pArray.array[gid].raw)
        {
            pArray.array[gid].raw = grzeroalloc<void*>(m_maxCmapGid+2);
            if (!pArray.array[gid].raw)
                return NULL; // malloc failed
            if (pArray.range[SEG_CACHE_MIN_INDEX] == SEG_CACHE_UNSET_INDEX)
            {
                pArray.range[SEG_CACHE_MIN_INDEX] = gid;
                pArray.range[SEG_CACHE_MAX_INDEX] = gid;
            }
            else
            {
                if (gid < pArray.range[SEG_CACHE_MIN_INDEX])
                    pArray.range[SEG_CACHE_MIN_INDEX] = gid;
                else if (gid > pArray.range[SEG_CACHE_MAX_INDEX])
                    pArray.range[SEG_CACHE_MAX_INDEX] = gid;
            }
        }
        pArray = pArray.array[gid];
        ++pos;
    }
    uint16 gid = (pos < length)? cmapGlyphs[pos] : 0;
    SegCachePrefixEntry * prefixEntry = pArray.prefixEntries[gid];
    if (!prefixEntry)
    {
        prefixEntry = new SegCachePrefixEntry();
        pArray.prefixEntries[gid] = prefixEntry;
        if (pArray.range[SEG_CACHE_MIN_INDEX] == SEG_CACHE_UNSET_INDEX)
        {
            pArray.range[SEG_CACHE_MIN_INDEX] = gid;
            pArray.range[SEG_CACHE_MAX_INDEX] = gid;
        }
        else
        {
            if (gid < pArray.range[SEG_CACHE_MIN_INDEX])
                pArray.range[SEG_CACHE_MIN_INDEX] = gid;
            else if (gid > pArray.range[SEG_CACHE_MAX_INDEX])
                pArray.range[SEG_CACHE_MAX_INDEX] = gid;
        }
    }
    if (!prefixEntry) return NULL;
    // if the cache is full run a purge - this is slow, since it walks the tree
    if (m_segmentCount + 1 > m_maxSegmentCount)
        purge();
    // if the cache is nearing full, try a purge on just the current prefixEntry
    //else if (m_segmentCount > .75 * m_maxSegmentCount)
    //{
    //    unsigned long long minAccessCount = m_totalAccessCount / (ePurgeFactor * m_maxSegmentCount);
    //    if (minAccessCount < 2) minAccessCount = 2;
    //    prefixEntry->purge(minAccessCount,
    //                       m_totalAccessCount - m_maxSegmentCount / 2,
    //                       m_totalAccessCount);
    //}
    ++m_segmentCount;
    return prefixEntry->cache(cmapGlyphs, length, seg, charOffset, m_totalAccessCount);
}

void SegCache::purge()
{
    unsigned long long minAccessCount = m_totalAccessCount / (ePurgeFactor * m_maxSegmentCount);
    if (minAccessCount < 2) minAccessCount = 2;
    purgeLevel(m_prefixes, 0, minAccessCount);
}

void SegCache::purgeLevel(SegCachePrefixArray prefixes, size_t level, unsigned long long minAccessCount)
{
    if (prefixes.range[SEG_CACHE_MIN_INDEX] == SEG_CACHE_UNSET_INDEX) return;
    size_t maxGlyphCached = prefixes.range[SEG_CACHE_MAX_INDEX];
    for (size_t i = prefixes.range[SEG_CACHE_MIN_INDEX]; i <= maxGlyphCached; i++)
    {
        if (prefixes.array[i].raw)
        {
            if (level + 1 < ePrefixLength)
                purgeLevel(prefixes.array[i], level + 1, minAccessCount);
            else
            {
                SegCachePrefixEntry * prefixEntry = prefixes.prefixEntries[i];
                m_segmentCount -= prefixEntry->purge(minAccessCount,
                    m_totalAccessCount - m_maxSegmentCount / 2, m_totalAccessCount);
            }
        }
    }
}

unsigned long long SegCachePrefixEntry::purge(unsigned long long minAccessCount,
                                              unsigned long long oldAccessTime,
                                              unsigned long long currentTime)
{
    // ignore the purge request if another has been done recently
    //if (m_lastPurge > oldAccessTime)
    //    return 0;

    long long totalPurged = 0;
    // real length is length + 1 in this loop
    for (uint16 length = 0; length < eMaxCachedSeg; length++)
    {
        if (m_entryCounts[length] == 0)
            continue;
        uint16 purgeIndices[eMaxSuffixCount];
        uint16 purgeCount = 0;
        for (uint16 j = 0; j < m_entryCounts[length]; j++)
        {
            SegCacheEntry & tempEntry = m_entries[length][j];
            // purge entries with a low access count which haven't been
            // accessed recently
            if (tempEntry.accessCount() < minAccessCount &&
                tempEntry.lastAccess() < oldAccessTime)
            {
                tempEntry.clear();
                purgeIndices[purgeCount++] = j;
            }
        }
        if (purgeCount == m_entryCounts[length])
        {
            m_entryCounts[length] = 0;
            m_entryBSIndex[length] = 0;
            free(m_entries[length]);
            m_entries[length] = NULL;
        }
        else if (purgeCount > 0)
        {
            uint16 oldIndex = 0;
            uint16 purgedCount = 0;
            uint16 newLength = m_entryCounts[length] - purgeCount;
            for (uint16 newIndex = 0; newIndex < newLength; newIndex++, oldIndex++)
            {
                while ((oldIndex == purgeIndices[purgedCount]) &&
                    (purgedCount < purgeCount))
                {
                    ++purgedCount;
                    ++oldIndex;
                }
                if (oldIndex > newIndex)
                    m_entries[length][newIndex] = m_entries[length][oldIndex];
            }
            m_entryCounts[length] = newLength;
        }
        totalPurged += purgeCount;
    }
    m_lastPurge = currentTime;
    return totalPurged;
}

}}}} // namespace
