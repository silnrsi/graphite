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
    : m_spaceGid(3), m_maxCmapGid(face_num_glyphs(face)),
    m_prefixLength(ePrefixLength),
    m_maxCachedSegLength(eMaxCachedSeg),
    m_segmentCount(0),
    m_maxSegmentCount(maxSegments),
    m_totalAccessCount(0l), m_totalMisses(0l),
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

void SegCache::freeLevel(void ** prefixes, size_t level)
{
    for (size_t i = 0; i < m_maxCmapGid; i++)
    {
        if (prefixes[i])
        {
            if (level + 1 < ePrefixLength)
                freeLevel((void**)prefixes[i], level + 1);
            else
            {
                SegCachePrefixEntry * prefixEntry = reinterpret_cast<SegCachePrefixEntry*>(prefixes[i]);
                delete prefixEntry;
            }
        }
    }
    free(prefixes);
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
    m_prefixes = NULL;
}

SegCacheEntry* SegCache::cache(const uint16* cmapGlyphs, size_t length, GrSegment * seg, size_t charOffset)
{
    uint16 pos = 0;
    if (!length) return NULL;
    assert(length < m_maxCachedSegLength);
    void ** pArray = m_prefixes;
    while (pos + 1 < m_prefixLength)
    {
        if (!pArray[(pos < length)? cmapGlyphs[pos] : 0])
            pArray[(pos < length)? cmapGlyphs[pos] : 0] = grzeroalloc<void*>(m_maxCmapGid);
        pArray = (void**)pArray[(pos < length)? cmapGlyphs[pos] : 0];
        ++pos;
    }

    SegCachePrefixEntry * prefixEntry = (SegCachePrefixEntry*)pArray[(pos < length)? cmapGlyphs[pos] : 0];
    if (!prefixEntry)
    {
        prefixEntry = new SegCachePrefixEntry();
        pArray[(pos < length)? cmapGlyphs[pos] : 0] = prefixEntry;
    }
    if (!prefixEntry) return NULL;
    ++m_segmentCount;
    return prefixEntry->cache(cmapGlyphs, length, seg, charOffset, m_totalAccessCount);
}

}}}} // namespace
