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
#include <graphite2/Segment.h>
#include "GrCachedFace.h"
#include "SegCacheStore.h"


/*virtual*/ GrCachedFace::~GrCachedFace()
{
    delete m_cacheStore;
}

bool GrCachedFace::setupCache(unsigned int cacheSize)
{
    m_cacheStore = new SegCacheStore(this, m_numSilf, cacheSize);
    return (m_cacheStore != NULL);
}


/*virtual*/ void GrCachedFace::runGraphite(GrSegment *seg, const Silf *pSilf) const
{
    assert(pSilf);
    pSilf->runGraphite(seg, 0, pSilf->substitutionPass());

    SegCache * segCache = NULL;
    unsigned int silfIndex = 0;

    for (unsigned int i = 0; i < m_numSilf; i++)
    {
        if (&(m_silfs[i]) == pSilf)
        {
            break;
        }
    }
    assert(silfIndex < m_numSilf);
    assert(m_cacheStore);
    segCache = m_cacheStore->getOrCreate(silfIndex, seg->getFeatures(0));
    // find where the segment can be broken
    GrSlot * subSegStartSlot = seg->first();
    GrSlot * subSegEndSlot = subSegStartSlot;
    uint16 cmapGlyphs[eMaxSpliceSize];
    int subSegStart = 0;
    bool spaceOnly = true;
    for (unsigned int i = 0; i < seg->charInfoCount(); i++)
    {
        if (i - subSegStart < eMaxSpliceSize)
        {
            cmapGlyphs[i-subSegStart] = subSegEndSlot->gid();
        }
        if (!m_cacheStore->isSpaceGlyph(subSegEndSlot->gid()))
        {
            spaceOnly = false;
        }
        // at this stage the character to slot mapping is still 1 to 1
        int breakWeight = seg->charinfo(i)->breakWeight();
        int nextBreakWeight = (i + 1 < seg->charInfoCount())?
            seg->charinfo(i+1)->breakWeight() : 0;
        if (((breakWeight > 0) &&
             (breakWeight <= gr_breakWord)) ||
            (i + 1 == seg->charInfoCount()) ||
             m_cacheStore->isSpaceGlyph(subSegEndSlot->gid()) ||
            ((i + 1 < seg->charInfoCount()) &&
             (((nextBreakWeight < 0) &&
              (nextBreakWeight >= gr_breakBeforeWord)) ||
              (subSegEndSlot->next() && m_cacheStore->isSpaceGlyph(subSegEndSlot->next()->gid())))))
        {
            // record the next slot before any splicing
            GrSlot * nextSlot = subSegEndSlot->next();
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
                    unsigned int length = i - subSegStart + 1;
                    SegmentScopeState scopeState = seg->setScope(subSegStartSlot, subSegEndSlot, length);
                    pSilf->runGraphite(seg, pSilf->substitutionPass(), pSilf->numPasses());
                    //entry =runGraphiteOnSubSeg(segCache, seg, cmapGlyphs,
                    //                           subSegStartSlot, subSegEndSlot,
                    //                           subSegStart, i - subSegStart + 1);
                    if ((length < eMaxSpliceSize) && segCache)
                        entry = segCache->cache(m_cacheStore, cmapGlyphs, length, seg, subSegStart);
                    seg->removeScope(scopeState);
                }
                else
                {
                    //seg->splice(subSegStart, i - subSegStart + 1, subSegStartSlot, subSegEndSlot, entry);
                    seg->splice(subSegStart, i - subSegStart + 1, subSegStartSlot, subSegEndSlot,
                        entry->first(), entry->glyphLength());
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

GRNG_EXPORT gr_face* gr_make_face_with_seg_cache(const void* appFaceHandle/*non-NULL*/, gr_get_table_fn getTable, unsigned int cacheSize, unsigned int faceOptions)
                  //the appFaceHandle must stay alive all the time when the GrFace is alive. When finished with the GrFace, call destroy_face
{
    GrCachedFace *res = new GrCachedFace(appFaceHandle, getTable);
#ifndef DISABLE_TRACING
    XmlTraceLog::get().openElement(ElementFace);
#endif
    bool valid = true;
    valid &= res->readGlyphs(faceOptions);
    if (!valid) {
        delete res;
        return 0;
    }
    valid &= res->readGraphite();
    valid &= res->readFeatures();
    valid &= res->setupCache(cacheSize);

#ifndef DISABLE_TRACING
    XmlTraceLog::get().closeElement(ElementFace);
#endif

    if (!(faceOptions & gr_face_dumbRendering) && !valid) {
        delete res;
        return 0;
    }
    return static_cast<gr_face *>(static_cast<GrFace *>(res));
}

#ifndef DISABLE_FILE_FACE

GRNG_EXPORT gr_face* gr_make_file_face_with_seg_cache(const char* filename, unsigned int segCacheMaxSize, unsigned int faceOptions)   //returns NULL on failure. //TBD better error handling
                  //when finished with, call destroy_face
{
    FileFace* pFileFace = new FileFace(filename);
    if (pFileFace->m_pTableDir)
    {
      gr_face* pRes = gr_make_face_with_seg_cache(pFileFace, &FileFace_table_fn, segCacheMaxSize, faceOptions);
      if (pRes)
      {
        pRes->takeFileFace(pFileFace);        //takes ownership
        return pRes;
      }
    }

    //error when loading

    delete pFileFace;
    return NULL;
}
#endif      //!DISABLE_FILE_FACE
