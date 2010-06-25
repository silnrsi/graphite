#include "graphiteng/SegmentHandle.h"
#include "Segment.h"
#include "graphiteng/ITextSource.h"

typedef unsigned int uchar_t;
namespace {
const int utf8_sz_lut[16] = {1,1,1,1,1,1,1,        // 1 byte
                                          0,0,0,0,  // trailing byte
                                          2,2,            // 2 bytes
                                          3,                 // 3 bytes
                                          4};                // 4 bytes
const byte utf8_mask_lut[5] = {0x80,0x00,0xC0,0xE0,0xF0};

inline uchar_t consume_utf8(const uint8 *&p) {
    const size_t    seq_sz = utf8_sz_lut[*p >> 4];
    uchar_t         uc = *p ^ utf8_mask_lut[seq_sz];
    
    switch(seq_sz) {
        case 4:     uc <<= 6; uc |= *++p & 0x7F;
        case 3:     uc <<= 6; uc |= *++p & 0x7F;
        case 2:     uc <<= 6; uc |= *++p & 0x7F; break;
        case 1:     break;
        case 0:     uc = 0xFFFD; break;
    }
    ++p; return uc;
}

const int SURROGATE_OFFSET = 0x10000 - (0xD800 << 10) - 0xDC00;
inline uchar_t consume_utf16(const uint16 *&p) {
    const uchar_t   uh = *p, ul = *++p;
    
    if (0xD800 > uh || uh > 0xDBFF)
        return uh;
    ++p;
    if (0xDC00 > ul || ul > 0xDFFF) {
        return 0xFFFD;
    }
    return (uh<<10) + ul - SURROGATE_OFFSET;
}

} // end of private namespace



static void read_text(const LoadedFace *face, const ITextSource *txt, Segment *seg, size_t numchars)
{
    const void *const   cmap = face->getTable(ktiCmap, NULL);
    const void *const   ctable = TtfUtil::FindCmapSubtable(cmap, 3, -1);
    const void *        pChar = txt->get_utf_buffer_begin();
    uchar_t             cid;
    unsigned int        gid;
    unsigned int	fid = seg->addFeatures(face->newFeatures(0));
    
    switch (txt->utfEncodingForm()) {
        case ITextSource::kutf8 : {
            const uint8 * p = static_cast<const uint8 *>(pChar);
            for (size_t i = 0; i < numchars; ++i) {
                cid = consume_utf8(p);
                gid = TtfUtil::Cmap31Lookup(ctable, cid);
                seg->appendSlot(i, cid, gid ? gid : face->findPseudo(cid), fid);
            }
            break;
        }
        case ITextSource::kutf16: {
            const uint16 * p = static_cast<const uint16 *>(pChar);
            for (size_t i = 0; i < numchars; ++i) {
                cid = consume_utf16(p);
                gid = TtfUtil::Cmap31Lookup(ctable, cid);
                seg->appendSlot(i, cid, gid ? gid : face->findPseudo(cid), fid);
            }
            break;
        }
        case ITextSource::kutf32 : default: {
            const uint32 * p = static_cast<const uint32 *>(pChar);
            for (size_t i = 0; i < numchars; ++i) {
                cid = *p++;
                gid = TtfUtil::Cmap31Lookup(ctable, cid);
                seg->appendSlot(i, cid, gid ? gid : face->findPseudo(cid), fid);
            }
            break;
        }
    }
}

static void prepare_pos(LoadedFont *font, Segment *seg)
{
    // reorder for bidi
    // copy key changeable metrics into slot (if any);
}

static void finalise(LoadedFont *font, Segment *seg)
{
    seg->positionSlots(font);
}


SegmentHandle::SegmentHandle(LoadedFont *font, const LoadedFace *face, const ITextSource *txt)
{
    int numchars = txt->getLength();
    m_pSegment = new Segment(numchars, face);

    m_pSegment->chooseSilf(0);
    read_text(face, txt, m_pSegment, numchars);
    m_pSegment->runGraphite();
    // run the line break passes
    // run the substitution passes
    prepare_pos(font, m_pSegment);
    // run the positioning passes
    finalise(font, m_pSegment);
#ifndef DISABLE_TRACING
    logSegment(*txt, *m_pSegment);
#endif
}


SegmentHandle::~SegmentHandle()
{
    delete m_pSegment;
}


int SegmentHandle::length() const
{
    return m_pSegment->length();
}


Position SegmentHandle::advance() const
{
    return m_pSegment->advance();
}


ISlot & SegmentHandle::operator[] (int index) const
{
    return m_pSegment->operator[](index);
}


void SegmentHandle::runGraphite()
{
    return m_pSegment->runGraphite();
}


void SegmentHandle::chooseSilf(uint32 script)
{
    return m_pSegment->chooseSilf(script);
}


