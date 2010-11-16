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
#include "SegCache.h"

namespace org { namespace sil { namespace graphite { namespace v2 {

class SegCache;
class GrFace;


class SegCacheStore
{
public:
    SegCacheStore(size_t maxSegments, uint32 flags) :
        m_caches(NULL), m_cacheCount(0), m_maxSegments(maxSegments), m_flags(flags)
    {
    };
    ~SegCacheStore()
    {
        for (size_t i = 0; i < m_cacheCount; i++)
        {
            delete m_caches[i];
        }
        free(m_caches);
        m_caches = NULL;
    }
    SegCache * getOrCreate(const GrFace * face, const Features & features)
    {
        for (size_t i = 0; i < m_cacheCount; i++)
        {
            if (m_caches[i]->features() == features)
                return m_caches[i];
        }
        SegCache ** newData = gralloc<SegCache*>(m_cacheCount+1);
        if (newData)
        {
            if (m_cacheCount > 0)
            {
                memcpy(newData, m_caches, sizeof(SegCache*) * m_cacheCount);
                free(m_caches);
            }
            m_caches = newData;
            m_caches[m_cacheCount] = new SegCache(face, features, m_maxSegments, m_flags);
            m_cacheCount++;
            return m_caches[m_cacheCount - 1];
        }
        return NULL;
    }

    CLASS_NEW_DELETE
private:
    SegCache ** m_caches;
    size_t m_maxSegments;
    size_t m_cacheCount;
    uint32 m_flags;
};

}}}}

