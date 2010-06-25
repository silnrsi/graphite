#include "Segment.h"
#include "Main.h"
#include "graphiteng/ITextSource.h"
#include "graphiteng/IFont.h"
#include "graphiteng/IFace.h"
#include "XmlTraceLog.h"
#include "LoadedFace.h"
#include "TtfUtil.h"
#include "LoadedFont.h"




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
    ++p;
    if (0xDC00 > ul || ul > 0xDFFF) {
        return 0xFFFD;
    }
    return (uh<<10) + ul - SURROGATE_OFFSET;
}

} // end of private namespace



