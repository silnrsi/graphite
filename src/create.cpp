#include "Segment.h"
#include "Main.h"
#include "graphiteng/ITextSource.h"
#include "graphiteng/IFont.h"
#include "graphiteng/IFace.h"
#include "XmlTraceLog.h"
#include "FontFace.h"
#include "TtfUtil.h"
#include "FontImpl.h"

#ifndef DISABLE_FILE_FONT
#include "FileFont.h"
#endif

void read_text(FontFace *font, const ITextSource *txt, Segment *seg, size_t numchars);
void prepare_pos(FontImpl *font, Segment *seg);
void finalise(FontImpl *font, Segment *seg);


#define GET_UTF8(p) \
    (*p < 0x80 ? *p++ : \
    (*p < 0xC0 ? p++, 0 : \
    (*p < 0xE0 ? ((unsigned int)(*p++ & 0x1F) << 6) + (*p++ & 0x3F) : \
    (*p < 0xF0 ? ((unsigned int)(*p++ & 0x0F) << 12) + ((*p++ & 0x3F) << 6) + (*p++ & 0x3F) : \
    (*p < 0xF8 ? ((unsigned int)(*p++ & 0x07) << 18) + ((*p++ & 0x3F) << 12) + ((*p++ & 0x3F) << 6) + (*p++ & 0x3F) : (*p++, 0))))))

#define GET_UTF16(p) \
    (*p < 0xDC00 && *p >= 0xD800 ? (p[1] >= 0xDC00 && p[1] < 0xE000 ? ((unsigned int)(*p++ & 0x3FF) << 10) + (*p++ & 0x3FF) : (p += 2, 0)) : *p++)

#define GET_UTF32(p) *p++


FontFace *create_fontface(IFace *face)
{
    FontFace *res = new FontFace(face);
    XmlTraceLog::get().openElement(ElementFace);
    if (res->readGlyphs() && res->readGraphite() && res->readFeatures())
    {
        XmlTraceLog::get().closeElement(ElementFace);
        return res;
    }
    XmlTraceLog::get().closeElement(ElementFace);
    delete res;
    return NULL;
}

class FileFontFace : public FontFace
{
public:
    FileFontFace(const char * filePath) : m_fileFont(filePath), FontFace(&m_fileFont) {}
private:
    FileFont m_fileFont;
};

FontFace *create_filefontface(const char *filePath)
{
    FontFace *res = new FileFontFace(filePath);
    XmlTraceLog::get().openElement(ElementFace);
    if (res->readGlyphs() && res->readGraphite() && res->readFeatures())
    {
        XmlTraceLog::get().closeElement(ElementFace);
        return res;
    }
    XmlTraceLog::get().closeElement(ElementFace);
    delete res;
    return NULL;
}

void destroy_fontface(FontFace *face)
{
    delete face;
}

// font my be NULL, but needs ppm in that case
FontImpl *create_font(IFont *font, FontFace *face, float ppm)
{
    FontImpl *res = new FontImpl(font, face, ppm);
    return res;
}

void destroy_font(FontImpl *font)
{
    delete font;
}


ISegment *create_rangesegment(FontImpl *font, FontFace *face, const ITextSource *txt)
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
    return seg;
}

void destroy_segment(ISegment *seg)
{
    delete seg;
}

/* Now we go private */

void read_text(FontFace *face, const ITextSource *txt, Segment *seg, size_t numchars)
{
    void *cmap = face->getTable(ktiCmap, NULL);
    void *ctable = TtfUtil::FindCmapSubtable(cmap, 3, -1);
    size_t form = txt->utfEncodingForm();
    const void *pBuffer = txt->get_utf_buffer_begin();
    const void *pChar = pBuffer;
    for (size_t i = 0; i < numchars; i++)
    {
        int cid;
        unsigned short gid;
        const uint8* pUChar;
        const uint16* pUShort;
        const uint32* pULong;

        switch (form)
        {   // this is oh so ugly
	case ITextSource::kutf8 :
            pUChar = reinterpret_cast<const uint8*>(pChar);
            cid = GET_UTF8(pUChar);
            pChar = pUChar;
            break;
	case ITextSource::kutf16 :
            pUShort = reinterpret_cast<const uint16*>(pChar);
            cid = GET_UTF16(pUShort);
            pChar = pUChar;
           break;
	case ITextSource::kutf32 :
        default:
            pULong = reinterpret_cast<const uint32*>(pChar);
            cid = GET_UTF32(pULong);
            pChar = pUChar;
	    break;
        }
        gid = TtfUtil::Cmap31Lookup(ctable, cid);
        if (!gid)
            gid = face->findPseudo(cid);
        seg->appendSlot(i, cid, gid);
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
