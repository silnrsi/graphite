#include "Segment.h"
#include "Misc.h"
#include "graphiteng/ITextSource.h"
#include "TtfUtil.h"
#include "FontFace.h"       // for table names

void read_text(IFont *font, ITextSource *txt, Segment *seg, int numchars);
void prepare_pos(IFont *font, Segment *seg);
void finalise(IFont *font, Segment *seg);


#define GET_UTF8(p) \
    (*p < 0x80 ? *p++ : \
    (*p < 0xC0 ? p++, 0 : \
    (*p < 0xE0 ? ((unsigned int)(*p++ & 0x1F) << 6) + (*p++ & 0x3F) : \
    (*p < 0xF0 ? ((unsigned int)(*p++ & 0x0F) << 12) + ((*p++ & 0x3F) << 6) + (*p++ & 0x3F) : \
    (*p < 0xF8 ? ((unsigned int)(*p++ & 0x07) << 18) + ((*p++ & 0x3F) << 12) + ((*p++ & 0x3F) << 6) + (*p++ & 0x3F) : (*p++, 0))))))

#define GET_UTF16(p) \
    (*p < 0xDC00 && *p >= 0xD800 ? (p[1] >= 0xDC00 && p[1] < 0xE000 ? ((unsigned int)(*p++ & 0x3FF) << 10) + (*p++ & 0x3FF) : (p += 2, 0)) : *p++)

#define GET_UTF32(p) *p++

ISegment *create_rangesegment(IFont *font, ITextSource *txt)
{
    int numchars = txt->getLength();
    Segment *seg = new Segment(numchars, font);

    read_text(font, txt, seg, numchars);
    // run the line break passes
    // run the substitution passes
    prepare_pos(font, seg);
    // run the positioning passes
    finalise(font, seg);
    return seg;
}

void read_text(IFont *font, ITextSource *txt, Segment *seg, int numchars)
{
    void *cmap = font->getTable(ktiCmap, NULL);
    void *ctable = TtfUtil::FindCmapSubtable(cmap, 3, -1);
    int form = txt->utfEncodingForm();
    void *pBuffer, *pChar;

    switch (form)
    {
    case kutf8 :
        pBuffer = reinterpret_cast<void *>(txt->get_utf8_buffer());
        break;
    case kutf16 :
        pBuffer = reinterpret_cast<void *>(txt->get_utf16_buffer());
        break;
    case kutf32 :
    default :
        pBuffer = reinterpret_cast<void *>(txt->get_utf32_buffer());
        break;
    }

    pChar = pBuffer;
    for (int i = 0; i < numchars; i++)
    {
        int cid;
        unsigned short gid;
        unsigned char *pUChar;
        unsigned short *pUShort;
        unsigned int *pULong;

        switch (form)
        {   // this is oh so ugly
        case kutf8 :
            pUChar = reinterpret_cast<unsigned char *>(pChar);
            cid = GET_UTF8(pUChar);
            pChar = reinterpret_cast<void *>(pUChar);
            break;
        case kutf16 :
            pUShort = reinterpret_cast<unsigned short *>(pChar);
            cid = GET_UTF16(pUShort);
            pChar = reinterpret_cast<void *>(pUShort);
            break;
        case kutf32 :
        default:
            pULong = reinterpret_cast<unsigned int *>(pChar);
            cid = GET_UTF32(pULong);
            pChar = reinterpret_cast<void *>(pULong);
            break;
        }
        gid = TtfUtil::Cmap31Lookup(ctable, cid);
        seg->initslots(i, cid, gid);
    }
}

void prepare_pos(IFont *font, Segment *seg)
{
    // reorder for bidi
    // copy key changeable metrics into slot (if any);
}

void finalise(IFont *font, Segment *seg)
{
    seg->positionSlots();
}
