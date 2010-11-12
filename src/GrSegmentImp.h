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

#include <cassert>

//#define USE_GRLIST
#ifdef USE_GRLIST
#include "GrList.h"
#else
#include <vector>
#endif

#include "SlotImp.h"
#include "CharInfoImp.h"
#include "FeaturesImp.h"
#include "XmlTraceLog.h"
#include "Silf.h"

namespace org { namespace sil { namespace graphite { namespace v2 {

#ifdef __GNUC__
// use the GNU CXX extension malloc_allocator to avoid new/delete
#include <ext/malloc_allocator.h>
typedef std::vector<Features, __gnu_cxx::malloc_allocator<Features> > FeatureList;
typedef std::vector<Slot*, __gnu_cxx::malloc_allocator<Slot*> > SlotRope;
typedef std::vector<uint16 *, __gnu_cxx::malloc_allocator<uint16 *> > AttributeRope;
#else
// standard allocator for other platforms
typedef std::vector<Features> FeatureList;
typedef std::vector<Slot *> SlotRope;
typedef std::vector<uint16 *> AttributeRope;
#endif

class SegmentScopeState;
class GrFace;
class GrSegment;

class SegmentScopeState
{
private:
    friend class GrSegment;
    Slot * realFirstSlot;
    Slot * slotBeforeScope;
    Slot * slotAfterScope;
    Slot * realLastSlot;
    size_t numGlyphsOutsideScope;
};

class GrSegment
{
public:
    unsigned int slotCount() const { return m_numGlyphs; }      //one slot per glyph
    void extendLength(int num) { m_numGlyphs += num; }
    Position advance() const { return m_advance; }
    void runGraphite() { if (m_silf) m_face->runGraphite(this, m_silf); };
    void chooseSilf(uint32 script) { m_silf = m_face->chooseSilf(script); }
    const Silf *silf() const { return m_silf; }
    unsigned int charInfoCount() const { return m_numCharinfo; }
    const CharInfo *charinfo(unsigned int index) const { return index < m_numCharinfo ? m_charinfo + index : NULL; }
    CharInfo *charinfo(unsigned int index) { return index < m_numCharinfo ? m_charinfo + index : NULL; }
    int8 dir() const { return m_dir; }

    GrSegment(unsigned int numchars, const GrFace* face, uint32 script, int dir);
    ~GrSegment();
    SegmentScopeState setScope(Slot * firstSlot, Slot * lastSlot, size_t subLength);
    void removeScope(SegmentScopeState & state);
    Slot *first() { return m_first; }
    void first(Slot *p) { m_first = p; }
    Slot *last() { return m_last; }
    void last(Slot *p) { m_last = p; }
    void appendSlot(int i, int cid, int gid, int fid);
    Slot *newSlot();
    void freeSlot(Slot *);
    void positionSlots(const GrFont *font, Slot *iStart = NULL, Slot *iEnd = NULL);
    void append(const GrSegment &other);
    uint16 getClassGlyph(uint16 cid, uint16 offset) const { return m_silf->getClassGlyph(cid, offset); }
    uint16 findClassIndex(uint16 cid, uint16 gid) const { return m_silf->findClassIndex(cid, gid); }
    int addFeatures(const Features& feats) { m_feats.push_back(feats); return m_feats.size() - 1; }
    uint16 getFeature(int index, uint8 findex) const { const FeatureRef* pFR=m_face->feature(findex); if (!pFR) return 0; else return pFR->getFeatureVal(m_feats[index]); }
    void dir(int8 val) { m_dir = val; }
    uint16 glyphAttr(uint16 gid, uint8 gattr) const { return m_face->glyphAttr(gid, gattr); }
    uint16 getGlyphMetric(Slot *iSlot, uint8 metric, uint8 attrLevel) const {
        if (attrLevel > 0)
        {
            Slot *is = findRoot(iSlot);
            return is->clusterMetric(this, metric, attrLevel);
        }
        else
            return m_face->getGlyphMetric(iSlot->gid(), metric);
    }
    float glyphAdvance(uint16 gid) const { return m_face->getAdvance(gid, 1.0); }
    const Rect &theGlyphBBoxTemporary(uint16 gid) const { return m_face->theBBoxTemporary(gid); }   //warning value may become invalid when another glyph is accessed
    Slot *findRoot(Slot *is) const { return is->attachTo() ? is : findRoot(is->attachTo()); }
    int numAttrs() { return m_silf->numUser(); }
    void splice(size_t offset, size_t length, Slot * startSlot, Slot * endSlot, const SegCacheEntry * entry);
    int defaultOriginal() const { return m_defaultOriginal; }
    const GrFace * getFace() const { return m_face; }

    CLASS_NEW_DELETE

#ifndef DISABLE_TRACING
    void logSegment(encform enc, const void* pStart, size_t nChars) const;
    void logSegment() const;
#endif

public:       //only used by: GrSegment* makeAndInitialize(const GrFont *font, const GrFace *face, uint32 script, const FeaturesHandle& pFeats/*must not be IsNull*/, encform enc, const void* pStart, size_t nChars, int dir);
    void read_text(const GrFace *face, const Features* pFeats/*must not be NULL*/, encform enc, const void*pStart, size_t nChars);
    void prepare_pos(const GrFont *font);
    void finalise(const GrFont *font);
  
private:
    SlotRope m_slots;           // std::vector of slot buffers
    Slot *m_freeSlots;          // linked list of free slots
    Slot *m_first;              // first slot in segment
    Slot *m_last;               // last slot in segment
    unsigned int m_bufSize;     // how big a buffer to create when need more slots
    unsigned int m_numGlyphs;
    int m_defaultOriginal;      // CharInfo index used if all slots have been deleted
    AttributeRope m_userAttrs;  // std::vector of userAttrs buffers
    CharInfo *m_charinfo;       // character info, one per input character
    unsigned int m_numCharinfo; // size of the array and number of input characters

    const GrFace *m_face;       // GrFace
    const Silf *m_silf;
    Position m_advance;         // whole segment advance
    Rect m_bbox;                // ink box of the segment
    int8 m_dir;
    FeatureList m_feats;	// feature settings referenced by charinfos in this segment

private:		//defensive on m_charinfo
    GrSegment(const GrSegment&);
    GrSegment& operator=(const GrSegment&);
};

}}}} // namespace
