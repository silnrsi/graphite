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
#include <cstdlib>
#include "graphiteng/CharInfo.h"
#include "Silf.h"
#include "XmlTraceLog.h"
#include "GrSegmentImp.h"
#include "SegCache.h"
#include "SegCacheStore.h"

using namespace org::sil::graphite::v2;

Silf::Silf() throw()
: m_passes(0), m_pseudos(0), m_classOffsets(0), m_classData(0),
  m_numPasses(0), m_sPass(0), m_pPass(0), m_jPass(0), m_bPass(0), m_flags(0),
  m_aBreak(0), m_aUser(0), m_iMaxComp(0),
  m_aLig(0), m_numPseudo(0), m_nClass(0), m_nLinear(0), m_segCacheStore(0)
{
}

Silf::~Silf() throw()
{
    releaseBuffers();
    if (m_segCacheStore)
    {
        delete m_segCacheStore;
        m_segCacheStore = NULL;
    }
}

void Silf::releaseBuffers() throw()
{
    delete [] m_passes;
    delete [] m_pseudos;
    free(m_classOffsets);
    free(m_classData);
    m_passes= 0;
    m_pseudos = 0;
    m_classOffsets = 0;
    m_classData = 0;
}


bool Silf::readGraphite(void *pSilf, size_t lSilf, int numGlyphs, uint32 version)
{
    byte *p = (byte *)pSilf;
    byte *eSilf = p + lSilf;
    uint32 *pPasses;
#ifndef DISABLE_TRACING
    XmlTraceLog::get().openElement(ElementSilfSub);
#endif
    if (version >= 0x00030000)
    {
#ifndef DISABLE_TRACING
        if (XmlTraceLog::get().active())
        {
            XmlTraceLog::get().addAttribute(AttrMajor, swap16(((uint16*) p)[0]));
            XmlTraceLog::get().addAttribute(AttrMinor, swap16(((uint16*) p)[1]));
        }
#endif
        if (lSilf < 27) { releaseBuffers(); return false; }
        p += 8;
    }
    else if (lSilf < 19) { releaseBuffers(); return false; }
    p += 2;     // maxGlyphID
    p += 4;     // extra ascent/descent
    m_numPasses = uint8(*p++);
#ifndef DISABLE_TRACING
    XmlTraceLog::get().addAttribute(AttrNumPasses, m_numPasses);
#endif
    if (m_numPasses > 128)
        return false;
    m_passes = new Pass[m_numPasses];
    m_sPass = uint8(*p++);
#ifndef DISABLE_TRACING
    XmlTraceLog::get().addAttribute(AttrSubPass, m_sPass);
#endif
    m_pPass = uint8(*p++);
#ifndef DISABLE_TRACING
    XmlTraceLog::get().addAttribute(AttrPosPass, m_pPass);
#endif
    if (m_pPass < m_sPass) {
        releaseBuffers();
        return false;
    }
    m_jPass = uint8(*p++);
#ifndef DISABLE_TRACING
    XmlTraceLog::get().addAttribute(AttrJustPass, m_jPass);
#endif
    if (m_jPass < m_pPass) {
        releaseBuffers();
        return false;
    }
    m_bPass = uint8(*p++);     // when do we reorder?
#ifndef DISABLE_TRACING
    XmlTraceLog::get().addAttribute(AttrBidiPass, m_bPass);
#endif
    if (m_bPass != 0xFF && (m_bPass < m_jPass || m_bPass > m_numPasses)) {
        releaseBuffers();
        return false;
    }
    m_flags = uint8(*p++);
    p += 2;     // ignore line end contextuals for now
    m_aPseudo = uint8(*p++);
    m_aBreak = uint8(*p++);
#ifndef DISABLE_TRACING
    XmlTraceLog::get().addAttribute(AttrBreakWeight, m_aBreak);
    XmlTraceLog::get().addAttribute(AttrDirectionality, *p);
#endif
    p++;        // we don't do bidi
    p += 2;     // skip reserved stuff
#ifndef DISABLE_TRACING
    XmlTraceLog::get().addAttribute(AttrNumJustLevels, *p);
#endif
    p += uint8(*p) * 8 + 1;     // ignore justification for now
    if (p + 9 >= eSilf) { releaseBuffers(); return false; }
    m_aLig = read16(p);
#ifndef DISABLE_TRACING
    XmlTraceLog::get().addAttribute(AttrLigComp, *p);
#endif
    if (m_aLig > 127) {
        releaseBuffers();
        return false;
    }
    m_aUser = uint8(*p++);
#ifndef DISABLE_TRACING
    XmlTraceLog::get().addAttribute(AttrUserDefn, m_aUser);
#endif
    m_iMaxComp = uint8(*p++);
#ifndef DISABLE_TRACING
    XmlTraceLog::get().addAttribute(AttrNumLigComp, m_iMaxComp);
#endif
    p += 5;     // skip direction and reserved
#ifndef DISABLE_TRACING
    XmlTraceLog::get().addAttribute(AttrNumCritFeatures, *p);
#endif
    p += uint8(*p) * 2 + 1;        // don't need critical features yet
    p++;        // reserved
    if (p >= eSilf) { releaseBuffers(); return false; }
#ifndef DISABLE_TRACING
    XmlTraceLog::get().addAttribute(AttrNumScripts, *p);
#endif
    p += uint8(*p) * 4 + 1;        // skip scripts
    p += 2;     // skip lbGID
    
    if (p + 4 * (m_numPasses + 1) + 6 >= eSilf) { releaseBuffers(); return false; }
    pPasses = (uint32 *)p;
    p += 4 * (m_numPasses + 1);
    m_numPseudo = read16(p);
#ifndef DISABLE_TRACING
    XmlTraceLog::get().addAttribute(AttrNumPseudo, m_numPseudo);
#endif
    p += 6;
    if (p + m_numPseudo * 6 >= eSilf) { releaseBuffers(); return false; }
    m_pseudos = new Pseudo[m_numPseudo];
    for (int i = 0; i < m_numPseudo; i++)
    {
        m_pseudos[i].uid = read32(p);
        m_pseudos[i].gid = read16(p);
#ifndef DISABLE_TRACING
        XmlTraceLog::get().openElement(ElementPseudo);
        XmlTraceLog::get().addAttribute(AttrIndex, i);
        XmlTraceLog::get().addAttribute(AttrGlyphId, m_pseudos[i].uid);
        XmlTraceLog::get().writeUnicode(m_pseudos[i].uid);
        XmlTraceLog::get().closeElement(ElementPseudo);
#endif
    }

    int clen = readClassMap((void *)p, swap32(*pPasses) - (p - (byte *)pSilf), numGlyphs + m_numPseudo);
    if (clen < 0) {
        releaseBuffers();
        return false;
    }
    p += clen;

    for (size_t i = 0; i < m_numPasses; ++i)
    {
        m_passes[i].init(this);
#ifndef DISABLE_TRACING
        if (XmlTraceLog::get().active())
        {
            XmlTraceLog::get().openElement(ElementPass);
            XmlTraceLog::get().addAttribute(AttrPassId, i);
        }
#endif
        if (!m_passes[i].readPass((char *)pSilf + swap32(pPasses[i]), swap32(pPasses[i + 1]) - swap32(pPasses[i]), swap32(pPasses[i])))
        {
#ifndef DISABLE_TRACING
            XmlTraceLog::get().closeElement(ElementPass);
#endif
            {
        releaseBuffers();
        return false;
    }
        }
#ifndef DISABLE_TRACING
        XmlTraceLog::get().closeElement(ElementPass);
#endif
    }
#ifndef DISABLE_TRACING
    XmlTraceLog::get().closeElement(ElementSilfSub);
#endif
    return true;
}

size_t Silf::readClassMap(void *pClass, size_t lClass, int numGlyphs)
{
    char *p = (char *)pClass;
    m_nClass = read16(p);
    m_nLinear = read16(p);
    m_classOffsets = gralloc<uint16>(m_nClass + 1);
#ifndef DISABLE_TRACING
    if (XmlTraceLog::get().active())
    {
        XmlTraceLog::get().openElement(ElementClassMap);
        XmlTraceLog::get().addAttribute(AttrNumClasses, m_nClass);
        XmlTraceLog::get().addAttribute(AttrNumLinear, m_nLinear);
    }
#endif

    for (int i = 0; i <= m_nClass; i++)
    {
        m_classOffsets[i] = read16(p) / 2 - (2 + m_nClass + 1);     // uint16[] index
    }

    if (m_classOffsets[0] != 0)
    {
#ifndef DISABLE_TRACING
        if (XmlTraceLog::get().active())
        {
            XmlTraceLog::get().error("Invalid first Class Offset %d expected %d",
                m_classOffsets[0], m_nLinear);
            XmlTraceLog::get().closeElement(ElementClassMap);
        }
#endif
        return -1;
    }
    if (m_classOffsets[m_nClass] + (2u + m_nClass + 1u) * 2 > lClass)
    {
#ifndef DISABLE_TRACING
        if (XmlTraceLog::get().active())
        {
            XmlTraceLog::get().error("Invalid Class Offset %d max size %d",
                m_classOffsets[m_nClass], lClass);
            XmlTraceLog::get().closeElement(ElementClassMap);
        }
#endif
        return -1;
    }
    m_classData = gralloc<uint16>(m_classOffsets[m_nClass]);
    for (int i = 0; i < m_classOffsets[m_nClass]; i++)
        m_classData[i] = read16(p);
#ifndef DISABLE_TRACING
    // TODO this includes extra checking which shouldn't be
    // disabled when tracing is inactive unless it is duplicated elsewhere
    if (XmlTraceLog::get().active())
    {
        bool glyphsOk = true;
        for (int i = 0; i < m_nClass; i++)
        {
            XmlTraceLog::get().openElement(ElementLookupClass);
            XmlTraceLog::get().addAttribute(AttrIndex, i);
            if (i < m_nLinear)
            {
                for (int j = m_classOffsets[i]; j < m_classOffsets[i+1]; j++)
                {
                    XmlTraceLog::get().openElement(ElementLookup);
                    XmlTraceLog::get().addAttribute(AttrGlyphId, m_classData[j]);
		    // out of range glyphids are allowed as place holders
                    // if (m_classData[j] >= numGlyphs)
                    // {
                    //     XmlTraceLog::get().warning("GlyphId out of range %d",
                    //         m_classData[j]);
                    //         glyphsOk = false;
                    // }
                    XmlTraceLog::get().closeElement(ElementLookup);
                }
            }
            else
            {
                int offset = m_classOffsets[i];
                uint16 numIds = m_classData[offset];
                XmlTraceLog::get().addAttribute(AttrNum, numIds);
                for (int j = offset + 4; j < m_classOffsets[i+1]; j += 2)
                {
                    XmlTraceLog::get().openElement(ElementLookup);
                    XmlTraceLog::get().addAttribute(AttrGlyphId, m_classData[j]);
                    XmlTraceLog::get().addAttribute(AttrIndex, m_classData[j+1]);
                    if (m_classData[j] >= numGlyphs)
                    {
                        XmlTraceLog::get().warning("GlyphId out of range %d",
                            m_classData[j]);
                    //         glyphsOk = false;
                    }
                    if (m_classData[j+1] >= numIds)
                    {
                       XmlTraceLog::get().warning("Index out of range %d",
                           m_classData[j+1]);
                           glyphsOk = false;
                    }
                    XmlTraceLog::get().closeElement(ElementLookup);
                }
            }
            XmlTraceLog::get().closeElement(ElementLookupClass);
        }
        XmlTraceLog::get().closeElement(ElementClassMap);
        if (!glyphsOk)
            return -1;
    }
#endif
    return (p - (char *)pClass);
}

uint16 Silf::findPseudo(uint32 uid) const
{
    for (int i = 0; i < m_numPseudo; i++)
        if (m_pseudos[i].uid == uid) return m_pseudos[i].gid;
    return 0;
}

uint16 Silf::findClassIndex(uint16 cid, uint16 gid) const
{
    if (cid > m_nClass) return -1;

#ifdef ENABLE_DEEP_TRACING
    if (XmlTraceLog::get().active())
    {
        XmlTraceLog::get().openElement(ElementLookupClass);
        XmlTraceLog::get().addAttribute(AttrNum, cid);
        XmlTraceLog::get().addAttribute(AttrGlyphId, gid);
    }
#endif

    uint16 loc = m_classOffsets[cid];
    if (cid < m_nLinear)        // output class being used for input, shouldn't happen
    {
        for (int i = loc; i < m_classOffsets[cid + 1]; i++)
            if (m_classData[i] == gid) return i - loc;
    }
    else
    {
        uint16 num = m_classData[loc];
        uint16 search = m_classData[loc + 1] << 1;
        uint16 selector = m_classData[loc + 2];
        uint16 range = m_classData[loc + 3];

        uint16 curr = loc + 4 + range * 2;

        while (search > 1)
        {
            int test;
            if (curr < loc + 4)
                test = -1;
            else
                test = m_classData[curr] - gid;

            if (test == 0)
            {
                uint16 res = m_classData[curr + 1];
#ifdef ENABLE_DEEP_TRACING
                XmlTraceLog::get().addAttribute(AttrIndex, res);
                XmlTraceLog::get().closeElement(ElementLookupClass);
#endif
                return res;
            }
            
            search >>= 1;
            if (test < 0)
                curr += search;
            else
                curr -= search;
        }
    }
#ifdef ENABLE_DEEP_TRACING
    XmlTraceLog::get().addAttribute(AttrIndex, -1);
    XmlTraceLog::get().closeElement(ElementLookupClass);
#endif
    return -1;
}

uint16 Silf::getClassGlyph(uint16 cid, int index) const
{
    if (cid > m_nClass) return 0;

    uint16 loc = m_classOffsets[cid];
    if (cid < m_nLinear)
    {
        if (index >= 0 && index < m_classOffsets[cid + 1] - loc)
            return m_classData[index + loc];
    }
    else        // input class being used for output. Shouldn't happen
    {
        for (int i = loc + 4; i < m_classOffsets[cid + 1]; i += 2)
            if (m_classData[i + 1] == index) return m_classData[i];
    }
    return 0;
}

void Silf::enableSegmentCache(const GrFace *face, size_t maxSegments, uint32 flags)
{
    if (!m_segCacheStore) m_segCacheStore = new SegCacheStore(face, maxSegments, flags);
}

void Silf::runGraphite(GrSegment *seg, const GrFace *face, VMScratch *vms) const
{
    if (m_segCacheStore)
    {
        runGraphiteWithCache(seg, face, vms);
        return;
    }
    for (size_t i = 0; i < m_numPasses; ++i)
    {
#ifndef DISABLE_TRACING
        if (XmlTraceLog::get().active())
        {
	        XmlTraceLog::get().openElement(ElementRunPass);
	        XmlTraceLog::get().addAttribute(AttrNum, i);
        }
#endif
        // test whether to reorder, prepare for positioning
        m_passes[i].runGraphite(seg, face, vms);
#ifndef DISABLE_TRACING
            seg->logSegment();
	    XmlTraceLog::get().closeElement(ElementRunPass);
#endif
    }
}

void Silf::runGraphiteWithCache(GrSegment *seg, const GrFace *face, VMScratch *vms) const
{
    // run up to substitution pass, i.e. run the line break passes
    for (size_t i = 0; i < m_sPass; ++i)
    {
#ifndef DISABLE_TRACING
        if (XmlTraceLog::get().active())
        {
            XmlTraceLog::get().openElement(ElementRunPass);
            XmlTraceLog::get().addAttribute(AttrNum, i);
        }
#endif
        // test whether to reorder, prepare for positioning
        m_passes[i].runGraphite(seg, face, vms);
#ifndef DISABLE_TRACING
        seg->logSegment();
        XmlTraceLog::get().closeElement(ElementRunPass);
#endif
    }

    SegCache * segCache = m_segCacheStore->getOrCreate(face, seg->getFeatures(0));
    // find where the segment can be broken
    Slot * subSegStartSlot = seg->first();
    Slot * subSegEndSlot = subSegStartSlot;
    uint16 cmapGlyphs[eMaxCachedSeg];
    int subSegStart = 0;
    bool spaceOnly = true;
    for (unsigned int i = 0; i < seg->charInfoCount(); i++)
    {
        if (i - subSegStart < eMaxCachedSeg)
        {
            cmapGlyphs[i-subSegStart] = subSegEndSlot->gid();
        }
        if (!m_segCacheStore->isSpaceGlyph(subSegEndSlot->gid()))
        {
            spaceOnly = false;
        }
        // at this stage the character to slot mapping is still 1 to 1
        int breakWeight = seg->charinfo(i)->breakWeight();
        int nextBreakWeight = (i + 1 < seg->charInfoCount())?
            seg->charinfo(i+1)->breakWeight() : 0;
        if (((breakWeight > 0) &&
             (breakWeight <= eBreakWord)) ||
            (i + 1 == seg->charInfoCount()) ||
             m_segCacheStore->isSpaceGlyph(subSegEndSlot->gid()) ||
            ((i + 1 < seg->charInfoCount()) &&
             (((nextBreakWeight < 0) &&
              (nextBreakWeight >= -eBreakWord)) ||
              (subSegEndSlot->next() && m_segCacheStore->isSpaceGlyph(subSegEndSlot->next()->gid())))))
        {
            // record the next slot before any splicing
            Slot * nextSlot = subSegEndSlot->next();
            if (spaceOnly)
            {
                // spaces should be left untouched by graphite rules in any sane font
            }
            else
            {
                // found a break position, check for a cache of the sub sequence
                const SegCacheEntry * entry = (segCache)?
                    segCache->find(cmapGlyphs, i - subSegStart + 1) : NULL;
                // TODO disable cache for words at start/end of line with contextuals
#ifndef DISABLE_TRACING
                if (XmlTraceLog::get().active())
                {
                    XmlTraceLog::get().openElement(ElementSubSeg);
                    XmlTraceLog::get().addAttribute(AttrFirstId, subSegStart);
                    XmlTraceLog::get().addAttribute(AttrLastId, i);
                }
#endif
                if (!entry)
                {
                    entry =runGraphiteOnSubSeg(segCache, seg, face, vms, cmapGlyphs,
                                               subSegStartSlot, subSegEndSlot,
                                               subSegStart, i - subSegStart + 1);
                }
                else
                {
                    seg->splice(subSegStart, i - subSegStart + 1, subSegStartSlot, subSegEndSlot, entry);
                }
#ifndef DISABLE_TRACING
                if (XmlTraceLog::get().active())
                {
                    XmlTraceLog::get().closeElement(ElementSubSeg);
                }
#endif
            }
            subSegEndSlot = nextSlot;
            subSegStartSlot = nextSlot;
            subSegStart = i + 1;
            spaceOnly = true;
        }
        else
        {
            subSegEndSlot = subSegEndSlot->next();
        }
    }
}

SegCacheEntry * Silf::runGraphiteOnSubSeg(SegCache* cache, GrSegment *seg, const GrFace *face,
                               VMScratch *vms, const uint16 * cmapGlyphs,
                               Slot * startSlot, Slot * endSlot,
                               size_t offset, size_t length) const
{

    //GrSegment subSeg(*seg, startSlot, offset, length);
    SegmentScopeState scopeState = seg->setScope(startSlot, endSlot, length);
    for (size_t i = m_sPass; i < m_numPasses; ++i)
    {
#ifndef DISABLE_TRACING
        if (XmlTraceLog::get().active())
        {
            XmlTraceLog::get().openElement(ElementRunPass);
            XmlTraceLog::get().addAttribute(AttrNum, i);
        }
#endif
        // test whether to reorder, prepare for positioning
        m_passes[i].runGraphite(seg, face, vms);
#ifndef DISABLE_TRACING
        seg->logSegment();
        XmlTraceLog::get().closeElement(ElementRunPass);
#endif
    }
    SegCacheEntry * entry = NULL;
    if (length < eMaxCachedSeg && cache)
        entry = cache->cache(m_segCacheStore, cmapGlyphs, length, seg, offset);
    seg->removeScope(scopeState);
    return entry;
}
