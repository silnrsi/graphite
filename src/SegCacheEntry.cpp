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
#include "Main.h"
#include "SlotImp.h"
#include "GrSegmentImp.h"
#include "SegCache.h"
#include "SegCacheEntry.h"

namespace org { namespace sil { namespace graphite { namespace v2 {

SegCacheEntry::SegCacheEntry(const uint16* cmapGlyphs, size_t length, GrSegment * seg, size_t charOffset, long long cacheTime)
    : m_glyphLength(0), m_unicode(gr2::gralloc<uint16>(length)), m_glyph(NULL),
    m_attr(NULL),
    m_accessCount(0), m_lastAccess(cacheTime)
{
    memcpy(m_unicode, cmapGlyphs, length * sizeof(*cmapGlyphs));
    size_t glyphCount = seg->slotCount();
    const Slot * slot = seg->first();
    m_glyph = new Slot[glyphCount];
    m_attr = gralloc<uint16>(glyphCount * seg->numAttrs());
    m_glyphLength = glyphCount;
    Slot * slotCopy = m_glyph;
    m_glyph->prev(NULL);
    static const char SLOT_PARENT = 1;
    static const char SLOT_SIBLING = 2;
    static const char SLOT_CHILD = 4;
    struct Index2Slot {
        Index2Slot(uint16 i, const Slot * s) : m_i(i), m_slot(s) {};
        Index2Slot() : m_i(0), m_slot(NULL) {};
        uint16 m_i;
        const Slot * m_slot;
    };
    struct Index2Slot parentGlyphs[eMaxCachedSeg];
    struct Index2Slot childGlyphs[eMaxCachedSeg];
    uint16 numParents = 0;
    uint16 numChildren = 0;
    uint16 pos = 0;
    while (slot)
    {
        slotCopy->userAttrs(m_attr + pos * seg->numAttrs());
        slotCopy->set(*slot, -charOffset, seg->numAttrs());
        if (slot->child())
        {
            new(parentGlyphs + numParents) Index2Slot( pos, slot );
            ++numParents;
        }
        if (slot->attachedTo())
        {
            new(childGlyphs + numChildren) Index2Slot( pos, slot );
            ++numChildren;
        }
        slot = slot->next();
        ++slotCopy;
        ++pos;
        if (slot)
        {
            slotCopy->prev(slotCopy-1);
            (slotCopy-1)->next(slotCopy);
        }
    }
    // loop over the attached children finding their siblings and parents
    for (int16 i = 0; i < numChildren; i++)
    {
        if (childGlyphs[i].m_slot->sibling())
        {
            for (int16 j = i; j < numChildren; j++)
            {
                if (childGlyphs[i].m_slot->sibling() == childGlyphs[j].m_slot)
                {
                    m_glyph[childGlyphs[i].m_i].sibling(m_glyph + childGlyphs[j].m_i);
                    break;
                }
            }
            if (!m_glyph[childGlyphs[i].m_i].sibling())
            {
                // search backwards
                for (int16 j = i-1; j >= 0; j--)
                {
                    if (childGlyphs[i].m_slot->sibling() == childGlyphs[j].m_slot)
                    {
                        m_glyph[childGlyphs[i].m_i].sibling(m_glyph + childGlyphs[j].m_i);
                        break;
                    }
                }
            }
        }
        // now find the parent glyph
        for (int16 j = 0; j < numParents; j++)
        {
            if (childGlyphs[i].m_slot->attachedTo() == parentGlyphs[j].m_slot)
            {
                m_glyph[childGlyphs[i].m_i].attachTo(m_glyph + parentGlyphs[j].m_i);
                if (parentGlyphs[j].m_slot->child() == childGlyphs[i].m_slot)
                {
                    m_glyph[parentGlyphs[j].m_i].child(m_glyph + childGlyphs[i].m_i);
                }
            }
        }
    }
}

void SegCacheEntry::log(size_t unicodeLength) const
{
#ifndef DISABLE_TRACING
    if (XmlTraceLog::get().active())
    {
        XmlTraceLog::get().openElement(ElementSegCacheEntry);
        XmlTraceLog::get().addAttribute(AttrAccessCount, m_accessCount);
        XmlTraceLog::get().addAttribute(AttrLastAccess, m_lastAccess);
        XmlTraceLog::get().addArrayElement(ElementText, m_unicode, unicodeLength);
        for (size_t i = 0; i < m_glyphLength; i++)
        {
            XmlTraceLog::get().openElement(ElementGlyph);
            XmlTraceLog::get().addAttribute(AttrGlyphId, m_glyph[i].gid());
            XmlTraceLog::get().addAttribute(AttrX, m_glyph[i].origin().x);
            XmlTraceLog::get().addAttribute(AttrY, m_glyph[i].origin().y);
            XmlTraceLog::get().addAttribute(AttrBefore, m_glyph[i].before());
            XmlTraceLog::get().addAttribute(AttrAfter, m_glyph[i].after());
            XmlTraceLog::get().closeElement(ElementGlyph);
        }
        XmlTraceLog::get().closeElement(ElementSegCacheEntry);
    }
#endif
}

void SegCacheEntry::clear()
{
    if (m_unicode) free(m_unicode);
    if (m_attr) free(m_attr);
    delete [] m_glyph;
    m_unicode = NULL;
    m_glyph = NULL;
    m_glyphLength = 0;
    m_attr = NULL;
}


}}}}
