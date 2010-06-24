#include "Segment.h"
#include "Main.h"
#include "graphiteng/ITextSource.h"
#include "graphiteng/IFont.h"
#include "graphiteng/IFace.h"
#include "XmlTraceLog.h"
#include "LoadedFace.h"
#include "TtfUtil.h"
#include "FontImpl.h"

void read_text(LoadedFace *font, const ITextSource *txt, Segment *seg, size_t numchars);
void prepare_pos(FontImpl *font, Segment *seg);
void finalise(FontImpl *font, Segment *seg);


// font my be NULL, but needs ppm in that case
FontImpl *create_font(IFont *font, LoadedFace *face, float ppm)
{
    FontImpl *res = new FontImpl(font, face, ppm);
    return res;
}

void destroy_font(FontImpl *font)
{
    delete font;
}


ISegment *create_rangesegment(FontImpl *font, LoadedFace *face, const ITextSource *txt)
{
    int numchars = txt->getLength();
    Segment *seg = new Segment(numchars, face);

    seg->chooseSilf(0);
    read_text(face, txt, seg, numchars);
    seg->runGraphite();
    // run the line break passes
    // run the substitution passes
    prepare_pos(font, seg);
    // run the positioning passes
    finalise(font, seg);
#ifndef DISABLE_TRACING
    logSegment(*txt, *seg);
#endif
    return seg;
}

void destroy_segment(ISegment *seg)
{
    delete seg;
}

/* Now we go private */
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
    if (0xDC00 > ul || ul > 0xDFFF) {
        ++p; return 0xFFFD;
    }
    return (uh<<10) + ul - SURROGATE_OFFSET;
}

} // end of private namespace


void read_text(LoadedFace *face, const ITextSource *txt, Segment *seg, size_t numchars)
{
    const void *const   cmap = face->getTable(ktiCmap, NULL);
    const void *const   ctable = TtfUtil::FindCmapSubtable(cmap, 3, -1);
    const void *        pChar = txt->get_utf_buffer_begin();
    uchar_t             cid;
    unsigned int        gid;
    
    switch (txt->utfEncodingForm()) {
        case ITextSource::kutf8 : {
            const uint8 * p = static_cast<const uint8 *>(pChar);
            for (size_t i = 0; i < numchars; ++i) {
                cid = consume_utf8(p);
                gid = TtfUtil::Cmap31Lookup(ctable, cid);
                seg->appendSlot(i, cid, gid ? gid : face->findPseudo(cid));
            }
            break;
        }
        case ITextSource::kutf16: {
            const uint16 * p = static_cast<const uint16 *>(pChar);
            for (size_t i = 0; i < numchars; ++i) {
                cid = consume_utf16(p);
                gid = TtfUtil::Cmap31Lookup(ctable, cid);
                seg->appendSlot(i, cid, gid ? gid : face->findPseudo(cid));
            }
            break;
        }
        case ITextSource::kutf32 : default: {
            const uint32 * p = static_cast<const uint32 *>(pChar);
            for (size_t i = 0; i < numchars; ++i) {
                cid = *p++;
                gid = TtfUtil::Cmap31Lookup(ctable, cid);
                seg->appendSlot(i, cid, gid ? gid : face->findPseudo(cid));
            }
            break;
        }
    }
}

void prepare_pos(FontImpl *font, Segment *seg)
{
    // reorder for bidi
    // copy key changeable metrics into slot (if any);
}

void finalise(FontImpl *font, Segment *seg)
{
    seg->positionSlots(font);
}
