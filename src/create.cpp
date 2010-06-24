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


#define GET_UTF8(p) \
    (*p < 0x80 ? *p++ : \
    (*p < 0xC0 ? p++, 0 : \
    (*p < 0xE0 ? ((unsigned int)(*p++ & 0x1F) << 6) + (*p++ & 0x3F) : \
    (*p < 0xF0 ? ((unsigned int)(*p++ & 0x0F) << 12) + ((*p++ & 0x3F) << 6) + (*p++ & 0x3F) : \
    (*p < 0xF8 ? ((unsigned int)(*p++ & 0x07) << 18) + ((*p++ & 0x3F) << 12) + ((*p++ & 0x3F) << 6) + (*p++ & 0x3F) : (*p++, 0))))))

#define GET_UTF16(p) \
    (*p < 0xDC00 && *p >= 0xD800 ? (p[1] >= 0xDC00 && p[1] < 0xE000 ? ((unsigned int)(*p++ & 0x3FF) << 10) + (*p++ & 0x3FF) : (p += 2, 0)) : *p++)

#define GET_UTF32(p) *p++


class FileFontFace
{
public:
    FileFontFace(const char * filePath) : m_pFileFont(IFace::loadTTFFile(filePath)), m_FontFace(m_pFileFont) {}
    
    const LoadedFace& theFontFace() const { return m_FontFace; }
    LoadedFace& theFontFace() { return m_FontFace; }

private:
    IFace* m_pFileFont;
    LoadedFace m_FontFace;
    
private:		//defensive on m_pFileFont
    FileFontFace(const FileFontFace&);
    FileFontFace& operator=(const FileFontFace&);
};

FileFontFace *create_filefontface(const char *filePath)
{
    FileFontFace *res2 = new FileFontFace(filePath);
    LoadedFace* res = &res2->theFontFace();
#ifndef DISABLE_TRACING
    XmlTraceLog::get().openElement(ElementFace);
#endif
    if (res->readGlyphs() && res->readGraphite() && res->readFeatures())
    {
#ifndef DISABLE_TRACING
        XmlTraceLog::get().closeElement(ElementFace);
#endif
        return res2;
    }
#ifndef DISABLE_TRACING
    XmlTraceLog::get().closeElement(ElementFace);
#endif
    delete res2;
    return NULL;
}

LoadedFace *the_fontface(FileFontFace *fileface)	//do not call destroy_fontface on this result
{
    return &fileface->theFontFace();
}


void destroy_filefontface(FileFontFace *fileface)
{
    delete fileface;
}


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

void read_text(LoadedFace *face, const ITextSource *txt, Segment *seg, size_t numchars)
{
    const void *const cmap = face->getTable(ktiCmap, NULL);
    const void *const ctable = TtfUtil::FindCmapSubtable(cmap, 3, -1);
    const void *      pChar = txt->get_utf_buffer_begin();
    unsigned int cid, gid;
    
    switch (txt->utfEncodingForm()) {
        case ITextSource::kutf8 : {
            const uint8 * p = static_cast<const uint8 *>(pChar);
            for (size_t i = 0; i < numchars; ++i) {
                cid = GET_UTF8(p);
                gid = TtfUtil::Cmap31Lookup(ctable, cid);
                seg->appendSlot(i, cid, gid ? gid : face->findPseudo(cid));
            }
            break;
        }
        case ITextSource::kutf16: {
            const uint16 * p = static_cast<const uint16 *>(pChar);
            for (size_t i = 0; i < numchars; ++i) {
                cid = GET_UTF16(p);
                gid = TtfUtil::Cmap31Lookup(ctable, cid);
                seg->appendSlot(i, cid, gid ? gid : face->findPseudo(cid));
            }
            break;
        }
        case ITextSource::kutf32 : default: {
            const uint32 * p = static_cast<const uint32 *>(pChar);
            for (size_t i = 0; i < numchars; ++i) {
                cid = GET_UTF32(p);
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
