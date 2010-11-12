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

#include "Main.h"

namespace org { namespace sil { namespace graphite { namespace v2 {

class GrSegment;

typedef enum {
    /** number of characters used in initial tree */
    ePrefixLength = 3,
    eMaxSuffixCount = 31,
    /** sub-Segments longer than this are not cached
        * (in Unicode code points) */
    eMaxCachedSeg = 24
} SegCacheLength;

/**
 * SegCacheEntry stores the result of running the engine for specific unicode
 * code points in the typical mid-line situation.
 */
class SegCacheEntry
{
    friend class SegCachePrefixEntry;
public:
    SegCacheEntry() :
        m_glyphLength(0), m_unicode(NULL), m_glyph(NULL), m_attr(NULL),
        m_accessCount(0), m_lastAccess(0)
    {}
    SegCacheEntry(const uint16 * cmapGlyphs, size_t length, GrSegment * seg, size_t charOffset, long long cacheTime);
    ~SegCacheEntry() { clear(); };
    void clear();
    size_t glyphLength() const { return m_glyphLength; }
    const Slot * first() const { ++m_accessCount; return m_glyph; }
    const Slot * last() const { return m_glyph + (m_glyphLength - 1); }

    void log(size_t unicodeLength) const;
    /** Total number of times this entry has been accessed since creation */
    long long accessCount() const { return m_accessCount; }
    /** "time" of last access where "time" is measured in accesses to the cache owning this entry */
    void accessed(unsigned long long cacheTime) const { m_lastAccess = cacheTime; ++m_accessCount; };

    CLASS_NEW_DELETE
private:

    size_t m_glyphLength;
    /** glyph ids resulting from cmap mapping from unicode to glyph before substitution
     * the length of this array is determined by the position in the SegCachePrefixEntry */
    uint16 * m_unicode;
    /** slots after shapping and positioning */
    Slot * m_glyph;
    uint16 * m_attr;
    mutable unsigned long long m_accessCount;
    mutable unsigned long long m_lastAccess;
};

}}}} // end namespace
