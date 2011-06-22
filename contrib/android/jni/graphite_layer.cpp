/*  GRAPHITE2 LICENSING

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

    Alternatively, you may use this library under the terms of the Mozilla
    Public License (http://mozilla.org/MPL) or under the GNU General Public
    License, as published by the Free Sofware Foundation; either version
    2 of the license or (at your option) any later version.
*/

#include "SkDraw.h"
#include "inject.h"
#include "ft2build.h"
#include FT_FREETYPE_H
#include FT_TRUETYPE_TABLES_H
#include "graphite2/Font.h"
#include "graphite2/Segment.h"
#include <stdlib.h>
#include <string.h>
#include "SkDescriptor.h"
#include "SkGlyphCache.h"
#include "SkBitmapProcShader.h"
#include "SkBlitter.h"
#include "SkBounder.h"
#include "SkDrawProcs.h"
#include "SkDrawFilter.h"
#include "SkScalerContext.h"
#include "SkCanvas.h"
#include "SkDevice.h"
#include "SkBitmap.h"
#include "SkTypeface.h"
#include <cutils/properties.h>

static int apilevel = 0;
int getapilevel()
{
    char buf[PROPERTY_VALUE_MAX + 1];
    property_get("ro.build.version.sdk", buf, "1");
    return strtoul(buf, NULL, 10);
}

class SkFaceRec;

// copy code from Skia, sigh. This from src/ports/SkFontHost_FreeType.cpp
class SkScalerContext_FreeType : public SkScalerContext {
public:
    SkScalerContext_FreeType(const SkDescriptor* desc);
    virtual ~SkScalerContext_FreeType();

    bool success() const {
        return fFaceRec != NULL &&
               fFTSize != NULL &&
               fFace != NULL;
    }

protected:
    virtual unsigned generateGlyphCount() const;
    virtual uint16_t generateCharToGlyph(SkUnichar uni);
    virtual void generateAdvance(SkGlyph* glyph);
    virtual void generateMetrics(SkGlyph* glyph);
    virtual void generateImage(const SkGlyph& glyph);
    virtual void generatePath(const SkGlyph& glyph, SkPath* path);
    virtual void generateFontMetrics(SkPaint::FontMetrics* mx,
                                     SkPaint::FontMetrics* my);
    virtual SkUnichar generateGlyphToChar(uint16_t glyph);

public:
    SkFaceRec*  fFaceRec;
    FT_Face     fFace;              // reference to shared face in gFaceRecHead
    FT_Size     fFTSize;            // our own copy
private:
    SkFixed     fScaleX, fScaleY;
    FT_Matrix   fMatrix22;
    uint32_t    fLoadGlyphFlags;

    FT_Error setupSize();
    void emboldenOutline(FT_Outline* outline);
};

// copied from skia/src/core/SkDraw.cpp
#define kStdStrikeThru_Offset       (-SK_Scalar1 * 6 / 21)
#define kStdUnderline_Offset        (SK_Scalar1 / 9)
#define kStdUnderline_Thickness     (SK_Scalar1 / 18)

static void draw_paint_rect(const SkDraw* draw, const SkPaint& paint,
                            const SkRect& r, SkScalar textSize) {
    if (paint.getStyle() == SkPaint::kFill_Style) {
        draw->drawRect(r, paint);
    } else {
        SkPaint p(paint);
        p.setStrokeWidth(SkScalarMul(textSize, paint.getStrokeWidth()));
        draw->drawRect(r, p);
    }
}

static void handle_aftertext(const SkDraw* draw, const SkPaint& paint,
                             SkScalar width, const SkPoint& start) {
    uint32_t flags = paint.getFlags();

    if (flags & (SkPaint::kUnderlineText_Flag |
                 SkPaint::kStrikeThruText_Flag)) {
        SkScalar textSize = paint.getTextSize();
        SkScalar height = SkScalarMul(textSize, kStdUnderline_Thickness);
        SkRect   r;

        r.fLeft = start.fX;
        r.fRight = start.fX + width;

        if (flags & SkPaint::kUnderlineText_Flag) {
            SkScalar offset = SkScalarMulAdd(textSize, kStdUnderline_Offset,
                                             start.fY);
            r.fTop = offset;
            r.fBottom = offset + height;
            draw_paint_rect(draw, paint, r, textSize);
        }
        if (flags & SkPaint::kStrikeThruText_Flag) {
            SkScalar offset = SkScalarMulAdd(textSize, kStdStrikeThru_Offset,
                                             start.fY);
            r.fTop = offset;
            r.fBottom = offset + height;
            draw_paint_rect(draw, paint, r, textSize);
        }
    }
}

// hack to deal with suprious SkToU8 in SkPaint.h. This should never get called.
unsigned char SkToU8(unsigned int x)
{ return 32; }

class mySkCanvas : public SkCanvas
{
public:
    explicit mySkCanvas(const SkBitmap& bitmap);
    explicit mySkCanvas(SkDevice *device = NULL);
    SkDevice* setDevice(SkDevice* device);
    virtual SkDevice* setBitmapDevice(const SkBitmap& bitmap);
    virtual SkDevice* createDevice(SkBitmap::Config config, int width, int height, bool isOpaque, bool isForLayer);
};

static mySkCanvas mySkCanvasDummy;

mySkCanvas::mySkCanvas(const SkBitmap& bitmap) :
    SkCanvas(bitmap)
{
    memcpy(this, &mySkCanvasDummy, sizeof(void *));
    setDevice(getDevice());
}

mySkCanvas::mySkCanvas(SkDevice *device) :
    SkCanvas(device)
{
    memcpy(this, &mySkCanvasDummy, sizeof(void *));
    setDevice(device);
}

SkDevice* mySkCanvas::setBitmapDevice(const SkBitmap& bitmap)
{
    return setDevice(SkCanvas::setBitmapDevice(bitmap));
}

#define LEVEL 8
#include "SkDevice_h.h"

#define LEVEL 9
#include "SkDevice_h.h"

typedef struct rec_ft_table {
    unsigned long tag;
    void *buffer;
    struct rec_ft_table *next;
} rec_ft_table;

typedef struct rec_gr_face {
    FT_Face ftface;
    gr_face *face;
    rec_ft_table *tables;
    struct rec_gr_face *next;
} rec_gr_face;

static rec_gr_face *rec_base = NULL;

void *gettable(const void *recp, unsigned int tag, size_t *len)
{
    rec_gr_face *rec = (rec_gr_face *)recp;
    rec_ft_table *r, *rlast;
    for (r = rec->tables, rlast = NULL; r; rlast = r, r = r->next)
    {
        if (r->tag == tag)
            return r->buffer;
    }

    r = new rec_ft_table;
    if (rlast)
        rlast->next = r;
    else
        rec->tables = r;
    r->next = NULL;
    r->tag = tag;

    unsigned long length = 0;
    FT_Load_Sfnt_Table(rec->ftface, tag, 0, NULL, &length);
    r->buffer = malloc(length);
    FT_Load_Sfnt_Table(rec->ftface, tag, 0, (FT_Byte *)(r->buffer), &length);
    *len = length;
    return r->buffer;
}

gr_face *getface(FT_Face ftface)
{
    rec_gr_face *r, *rlast;
    for (r = rec_base, rlast = NULL; r; rlast = r, r = r->next)
        if (r->ftface == ftface) return r->face;

    r = new rec_gr_face;
    r->ftface = ftface;
    r->tables = NULL;
    r->next = NULL;
    r->face = gr_make_face(r, (gr_get_table_fn)&gettable, 0);
    if (rlast)
        rlast->next = r;
    else
        rec_base = r;
    return r->face;
}

class mySkDraw : public SkDraw
{
public:
    void drawText(const char *text, size_t bytelen, SkScalar x, SkScalar y, const SkPaint& paint) const;
};

SkDevice *mySkCanvas::setDevice(SkDevice* device)
{
    if (device)
    {
        if (!apilevel) apilevel = getapilevel();
        if (apilevel > 8)
            mySkDevice9 *myDevice = new mySkDevice9(device->accessBitmap(0));
        else
            mySkDevice8 *myDevice = new mySkDevice8(device->accessBitmap(0));
        return SkCanvas::setDevice((SkDevice *)myDevice);
    }
    else
        return device;
}

SkDevice* mySkCanvas::createDevice(SkBitmap::Config config, int width, int height, bool isOpaque, bool isForLayer)
{
    SkDevice *temp = SkCanvas::createDevice(config, width, height, isOpaque, isForLayer);
    if (!apilevel) apilevel = getapilevel();
    if (apilevel > 8)
        mySkDevice9 *res = new mySkDevice9(temp->accessBitmap(0));
    else
        mySkDevice8 *res = new mySkDevice8(temp->accessBitmap(0));
    delete temp;
    return (SkDevice *)res;
}

gr_face* getface_from_paint(const SkPaint &paint)
{
    SkScalerContext::Rec rec;
    memset(&rec, 0, sizeof(rec));
    rec.fFontID = SkTypeface::UniqueID(paint.getTypeface());
    rec.fTextSize = paint.getTextSize();
    rec.fPreScaleX = paint.getTextScaleX();
    rec.fPreSkewX = paint.getTextSkewX();
    SkAutoDescriptor    ad(sizeof(rec) + SkDescriptor::ComputeOverhead(1));
    SkDescriptor *      desc = ad.getDesc();
    desc->init();
    desc->addEntry(kRec_SkDescriptorTag, sizeof(rec), &rec);
    SkScalerContext_FreeType    *context = static_cast<SkScalerContext_FreeType *>(SkScalerContext::Create(desc));
    
//    FT_Face *ftface = *((char *)context + offset(SkScalerContext_FreeType::fFace));     // we are so evil ;)
    FT_Face ftface = context->fFace;
    return getface(ftface);
}

// mangled name: _ZNK8mySkDraw8drawTextEPKcjffRK7SkPaint 
void mySkDraw::drawText(const char *text, size_t bytelen, SkScalar x, SkScalar y, const SkPaint& paint) const
{
    // encapsulation is evil!!
    // we have to create a SkDescriptor so that we can create a SkScalerContext_FreeType
    // so that we can break its encapsulation to get at the FT_Face
    gr_face *face = getface_from_paint(paint);
    gr_encform enctype = gr_encform(paint.getTextEncoding() + 1);
    if (fMatrix->getType() & SkMatrix::kPerspective_Mask || enctype > 2 || !face)
    {
        SkDraw::drawText(text, bytelen, x, y, paint);
        return;
    }

    if (text == NULL || bytelen == 0 || fClip->isEmpty() ||
            (paint.getAlpha() == 0 && paint.getXfermode() == NULL))
        return;

    // we don't recalculate this because the values may be set by a call to SkScalarContext_FreeType::setupSize()
//    FT_Size ftsize = *((char *)context + offset(SkScalerContext_FreeType::fFTSize));    // we are so evil ;)
    gr_font *font = gr_make_font(paint.getTextSize(), face); // textsize in pixels
    if (!font) return;

    size_t numchar = gr_count_unicode_characters(enctype, text, text + bytelen, NULL);
    gr_segment *seg = gr_make_seg(font, face, 0, 0, enctype, text, numchar, 0);
    if (!seg)
    {
        gr_font_destroy(font);
        return;
    }

    SkScalar underlineWidth = 0;
    SkPoint  underlineStart;
    SkPoint  segWidth;

    fMatrix->mapXY(gr_seg_advance_X(seg), gr_seg_advance_Y(seg), &segWidth);
    underlineStart.set(0, 0);
    if (paint.getFlags() & (SkPaint::kUnderlineText_Flag | SkPaint::kStrikeThruText_Flag))
    {
        underlineWidth = segWidth.fX;
        SkScalar offsetX = 0;
        if (paint.getTextAlign() == SkPaint::kCenter_Align)
            offsetX = SkScalarHalf(underlineWidth);
        else if (paint.getTextAlign() == SkPaint::kRight_Align)
            offsetX = underlineWidth;
        underlineStart.set(x - offsetX, y);
    }

    SkAutoGlyphCache    autoCache(paint, fMatrix);
    SkGlyphCache*       cache = autoCache.getCache();
    unsigned int bitmapstore[sizeof(SkBitmapProcShader) >> 2];
    SkBlitter*  blitter = SkBlitter::Choose(*fBitmap, *fMatrix, paint,
                                bitmapstore, sizeof(bitmapstore));

    if (paint.getTextAlign() == SkPaint::kRight_Align)
    {
        x -= segWidth.fX;
        y -= segWidth.fY;       // do we top align too?
    }
    else if (paint.getTextAlign() == SkPaint::kCenter_Align)
    {
        x -= SkScalarHalf(segWidth.fX);
        y -= SkScalarHalf(segWidth.fY);
    }

    SkDraw1Glyph        dlg;
    SkDraw1Glyph::Proc  proc = dlg.init(this, blitter, cache);
    const gr_slot *s;
    for (s = gr_seg_first_slot(seg); s; s = gr_slot_next_in_segment(s))
    {
        SkPoint pos;
        fMatrix->mapXY(gr_slot_origin_X(s) + x, -gr_slot_origin_Y(s) + y, &pos);
        const SkGlyph& glyph = cache->getGlyphIDMetrics(gr_slot_gid(s));
        if (glyph.fWidth)
            proc(dlg, glyph, SkFixedFloor(SkScalarToFixed(pos.fX)), SkFixedFloor(SkScalarToFixed(pos.fY)));
    }

    if (underlineWidth)
    {
        autoCache.release();
        handle_aftertext(this, paint, underlineWidth, underlineStart);
    }
    gr_seg_destroy(seg);
    gr_font_destroy(font);
}

class mySkPaint : public SkPaint
{
public:
    SkScalar    measureText(const void* text, size_t length,
                            SkRect* bounds, SkScalar scale = 0) const;
    SkScalar    measureText(const void* text, size_t length) const;
    int         getTextWidths(const void* text, size_t byteLength, SkScalar widths[],
                            SkRect bounds[] = NULL) const;
};

SkScalar mySkPaint::measureText(const void *text, size_t length) const
{
    return mySkPaint::measureText(text, length, NULL, 0);
}

SkScalar mySkPaint::measureText(const void* textData, size_t length, SkRect *bounds, SkScalar zoom) const
{
    const char* text = (const char *)textData;
    gr_face *face = getface_from_paint(*this);
    gr_encform enctype = gr_encform(getTextEncoding() + 1);
    if (enctype > 2 || !face)
        return SkPaint::measureText(text, length, bounds, zoom);
    gr_font *font = gr_make_font(zoom ? SkScalarMul(getTextSize(), zoom) : getTextSize(), face);
    if (!font) return 0;

    size_t numchar = gr_count_unicode_characters(enctype, text, text + length, NULL);
    gr_segment *seg = gr_make_seg(font, face, 0, 0, enctype, text, numchar, 0);
    if (!seg)
    {
        gr_font_destroy(font);
        return 0;
    }
    SkScalar width = gr_seg_advance_X(seg);
    if (bounds)
    {
        bounds->fLeft = 0;
        bounds->fBottom = 0;
        bounds->fRight = width;
        bounds->fTop = gr_seg_advance_Y(seg);
    }
    gr_seg_destroy(seg);
    gr_font_destroy(font);
    return width;
}

int mySkPaint::getTextWidths(const void* textData, size_t byteLength, SkScalar widths[], SkRect bounds[]) const
{
    const char* text = (const char *)textData;
    gr_face *face = getface_from_paint(*this);
    gr_encform enctype = gr_encform(getTextEncoding() + 1);
    if (enctype > 2 || !face)
        return SkPaint::getTextWidths(textData, byteLength, widths, bounds);
    gr_font *font = gr_make_font(getTextSize(), face);
    if (!font) return 0;

    size_t numchar = gr_count_unicode_characters(enctype, text, text + byteLength, NULL);
    gr_segment *seg = gr_make_seg(font, face, 0, 0, enctype, text, numchar, 0);
    if (!seg)
    {
        gr_font_destroy(font);
        return 0;
    }
    float width = 0;
    for (int i = 0; i < numchar; ++i)
    {
        const gr_char_info *c = gr_seg_cinfo(seg, i);
        int a = gr_cinfo_after(c);
        int b = gr_cinfo_before(c);
        const gr_slot *s;
        const gr_slot *as = NULL, *bs = NULL;
        for (s = gr_seg_first_slot(seg); s; s = gr_slot_next_in_segment(s))
        {
            if (gr_slot_index(s) == a)
            {
                as = s;
                if (bs) break;
            }
            else if (gr_slot_index(s) == b)
            {
                bs = s;
                if (as) break;
            }
        }
        *widths = (as ? gr_slot_origin_X(as) : gr_seg_advance_X(seg)) - width;
        if (bounds)
        {
            bounds->fLeft = width;
            bounds->fRight = *widths + width;
            bounds->fBottom = 0;
            bounds->fTop = 0;
            ++bounds;
        }
        width += *widths;
        ++widths;
    }
    return numchar;
}


extern myfontmap *myfonts;

class mySkTypeface : public SkTypeface
{
public:
    static SkTypeface *CreateFromName(const char name[], SkTypeface::Style style);
};

SkTypeface *mySkTypeface::CreateFromName(const char name[], SkTypeface::Style style)
{
    myfontmap *f;
    for (f = myfonts; f; f = f->next)
    {
        if (!strcmp(name, f->name))
        {
            f->tf->safeRef();
            return f->tf;
        }
    }
    return SkTypeface::CreateFromName(name, style);
}


func_map thismap[] = {
    // SkDraw::DrawText,                            mySkDraw::DrawText
    { "_ZNK6SkDraw8drawTextEPKcjffRK7SkPaint",      "_ZNK8mySkDraw8drawTextEPKcjffRK7SkPaint", 0, 0 },
    // SkCanvas::SkCanvas(SkBitmap&),               mySkCanvas::mySkCanvas(SkBitmap&)
    { "_ZN8SkCanvasC1ERK8SkBitmap",                 "_ZN10mySkCanvasC1ERK8SkBitmap", 0, 0 },
    { "_ZN8SkCanvasC2ERK8SkBitmap",                 "_ZN10mySkCanvasC2ERK8SkBitmap", 0, 0 },
    // SkCanvas::SkCanvas(SkDevice*),               mySkCanvas::mySkCanvas(SkDevice*)
    { "_ZN8SkCanvasC1EP8SkDevice",                  "_ZN10mySkCanvasC1EP8SkDevice", 0, 0 },
    { "_ZN8SkCanvasC2EP8SkDevice",                  "_ZN10mySkCanvasC2EP8SkDevice", 0, 0 },
    // SkCanvas::setBitmapDevice,                   mySkCanvas::setBitmapDevice
    { "_ZN8SkCanvas15setBitmapDeviceERK8SkBitmap",  "_ZN10mySkCanvas15setBitmapDeviceERK8SkBitmap", 0, 0 },
    // SkCanvas::setDevice,                         mySkCanvas::setDevice
    { "_ZN8SkCanvas9setDeviceEP8SkDevice",          "_ZN10mySkCanvas9setDeviceEP8SkDevice", 0, 0 },
    // SkDevice::drawText                                mySkDraw::drawText
    { "_ZN8SkDevice8drawTextERK6SkDrawPKvjffRK7SkPaint", "_ZN10mySkDevice8drawTextERK6SkDrawPKvjffRK7SkPaint", 0, 0 },
    // SkTypeface::CreateFromName                        mySkTypeface::CreateFromName
    { "_ZN10SkTypeface14CreateFromNameEPKcNS_5StyleE",   "_ZN12mySkTypeface14CreateFromNameEPKcN10SkTypeface5StyleE", 0, 0 },
    // SkPaint::measureText                         SkPaint::measureText
    { "_ZNK7SkPaint11measureTextEPKvjP6SkRectf",    "_ZNK9mySkPaint11measureTextEPKvjP6SkRectf", 0, 0 },
    // SkPaint::measureText                         SkPaint::measureText
    { "_ZNK7SkPaint11measureTextEPKvj",             "_ZNK9mySkPaint11measureTextEPKvj", 0, 0},
    // SkPaint::getTextWidths
    { "_ZNK7SkPaint13getTextWidthsEPKvjPfP6SkRect", "_ZNK9mySkPaint13getTextWidthsEPKvjPfP6SkRect", 0, 0}
};

