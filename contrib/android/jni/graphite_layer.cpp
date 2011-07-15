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

#include "load.h"
#include "load_gr.h"
#include "SkDraw.h"
#include "graphite2/Font.h"
#include "graphite2/Segment.h"
#include <stdlib.h>
#include <string.h>
#include "SkGlyphCache.h"
#include "SkBitmapProcShader.h"
#include "SkBlitter.h"
#include "SkDrawProcs.h"
#include "SkCanvas.h"
#include "SkDevice.h"
#include "SkBitmap.h"

class SkFaceRec;

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

// local classes
class mySkCanvas : public SkCanvas
{
public:
    explicit mySkCanvas(const SkBitmap& bitmap);
    explicit mySkCanvas(SkDevice *device = NULL);
    SkDevice* setDevice(SkDevice* device);
    virtual SkDevice* setBitmapDevice(const SkBitmap& bitmap);
    virtual SkDevice* createDevice(SkBitmap::Config config, int width, int height, bool isOpaque, bool isForLayer);
};

class newSkCanvas : public SkCanvas
{
public:
	newSkCanvas(SkDevice *device = NULL) {};
};

class mySkDraw : public SkDraw
{
public:
    void drawText(const char *text, size_t bytelen, SkScalar x, SkScalar y, const SkPaint& paint) const;
};

class mySkDevice : public SkDevice
{
public:
    virtual void drawText(const SkDraw& d, const void *text, size_t len, SkScalar x, SkScalar y, const SkPaint &paint);
};

class newSkDevice : public SkDevice {};

class mySkTypeface : public SkTypeface
{
public:
    static SkTypeface *CreateFromName(const char name[], SkTypeface::Style style);
};

static mySkCanvas mySkCanvasDummy((SkDevice *)1);
static newSkCanvas newSkCanvasDummy((SkDevice *)1);
static mySkDevice mySkDeviceDummy;
static newSkDevice newSkDeviceDummy;

// functions
void hookvtbl(void *dest, void *base, void *sub, int num)
{
    ptrdiff_t *d = *(ptrdiff_t **)dest;		// long
    ptrdiff_t *b = *(ptrdiff_t **)base;		// short
    ptrdiff_t *s = *(ptrdiff_t **)sub;		// short
    ptrdiff_t *newv = (ptrdiff_t *)malloc(num * sizeof(ptrdiff_t));
    int j = 2, i, k, l;

    // handle destructors
    for (i = 0; i < 2; ++i)
    	newv[i] = d[i];

    for (i = 2; i < num; ++i)
    {
        if (d[i] == b[j])
        {
            newv[i] = s[j];
            ++j;
        }
        else
        {
            newv[i] = d[i];
            for (k = i + 1, l = j + 1; k < num; ++k, ++l)
            {
                if (d[k] == b[l])
                {
                    newv[i] = s[j];
                    i = k - 1;
                    j = l;
                    break;
                }
                else
                    newv[k] = s[l];
                if (!d[k] || !b[l]) break;
            }
        }
        if (!d[i]) break;
    }
    *(ptrdiff_t **)dest = newv;
}

mySkCanvas::mySkCanvas(const SkBitmap& bitmap) :
    SkCanvas(bitmap)
{
    hookvtbl(this, &newSkCanvasDummy, &mySkCanvasDummy, 62);    // 38 + 1 rtti? + 2 destructor + spare
    setDevice(getDevice());
}

mySkCanvas::mySkCanvas(SkDevice *device) :
    SkCanvas(device == (SkDevice *)1 ? NULL : device)
{
    if (device != (SkDevice *)1)
        hookvtbl(this, &newSkCanvasDummy, &mySkCanvasDummy, 62);
    else
        device = NULL;
    setDevice(device);
}

SkDevice* mySkCanvas::setBitmapDevice(const SkBitmap& bitmap)
{
    return setDevice(SkCanvas::setBitmapDevice(bitmap));
}

void mySkDevice::drawText(const SkDraw& d, const void *text, size_t len, SkScalar x, SkScalar y, const SkPaint &paint)
{
    ((mySkDraw *)(&d))->drawText((const char *)text, len, x, y, paint);
}

SkDevice *mySkCanvas::setDevice(SkDevice* device)
{
    if (device)
        hookvtbl(device, &newSkDeviceDummy, &mySkDeviceDummy, 41);    // 17 + spares
    return SkCanvas::setDevice(device);
}

SkDevice* mySkCanvas::createDevice(SkBitmap::Config config, int width, int height, bool isOpaque, bool isForLayer)
{
    SkDevice *res = SkCanvas::createDevice(config, width, height, isOpaque, isForLayer);
    hookvtbl(res, &newSkDeviceDummy, &mySkDeviceDummy, 41);    // 17 + spares
    return res;
}

// mangled name: _ZNK8mySkDraw8drawTextEPKcjffRK7SkPaint 
void mySkDraw::drawText(const char *text, size_t bytelen, SkScalar x, SkScalar y, const SkPaint& paint) const
{
    gr_face *face = gr_face_from_tf(paint.getTypeface());
    gr_encform enctype = gr_encform(paint.getTextEncoding() + 1);
    if (fMatrix->getType() & SkMatrix::kPerspective_Mask || enctype > 2 || !face)
    {
        SkDraw::drawText(text, bytelen, x, y, paint);
        return;
    }

    if (text == NULL || bytelen == 0 || fClip->isEmpty() ||
            (paint.getAlpha() == 0 && paint.getXfermode() == NULL))
        return;

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
    gr_face *face = gr_face_from_tf(getTypeface());
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
    gr_face *face = gr_face_from_tf(getTypeface());
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

SkTypeface *mySkTypeface::CreateFromName(const char name[], SkTypeface::Style style)
{
    SkTypeface *res = tf_from_name(name);
    if (res)
        return res;
    else
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

