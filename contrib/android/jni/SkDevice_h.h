/*
 * Derived from android/external/skia/include/core/SkDevice.h
 *
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include "SkRefCnt.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkColor.h"

class SkDraw;
struct SkIRect;
class SkMatrix;
class SkRegion;

#define _incfn(type, name, decl, params) virtual type name decl { return SkDevice::name params; }
#define _incfnc(type, name, decl, params) virtual type name decl const { return SkDevice::name params; }
#define _incfnv(name, decl, params) virtual void name decl { SkDevice::name params; }
#define _ignorefn(type, name, decl, params) extern type SkDevice::name decl __attribute__((weak_import))
#define _ignorefnv(name, decl, params) extern void SkDevice::name decl __attribute__((weak_import))
#define _ignorefnc(type, name, decl, params) extern type SkDevice::name decl const __attribute__((weak_import))
#define _concat__(x, y) x##y
#define _concat(x, y) _concat__(x, y)
#define SKCLASS(x) _concat(x, LEVEL)

#if LEVEL == 8
#define _l8(type, name, decl, params) _incfn(type, name, decl params)
#define _l8v(name, decl, params) _incfnv(name, decl, params)
#define _l8c(type, name, decl, params) _incfnc(type, name, decl params)
#define _l9(type, name, decl, params) _ignorefn(type, name, decl, params)
#define _l9v(name, decl, params) _ignorefnv(name, decl, params)
#define _l9c(type, name, decl, params) _ignorefnc(type, name, decl, params)
#endif

#if LEVEL == 9 || LEVEL == 10
#define _l9(type, name, decl, params) _incfn(type, name, decl, params)
#define _l9v(name, decl, params) _incfnv(name, decl, params)
#define _l9c(type, name, decl, params) _incfnc(type, name, decl, params)
#endif

class SKCLASS(SkDevice) : public SkRefCnt {
public:
    SKCLASS(SkDevice)();
    SKCLASS(SkDevice)(const SkBitmap& bitmap);

    int width() const { return fBitmap.width(); }
    int height() const { return fBitmap.height(); }
    SkBitmap::Config config() const { return fBitmap.getConfig(); }
    bool isOpaque() const { return fBitmap.isOpaque(); }
    
    void getBounds(SkIRect* bounds) const;
    
    bool intersects(const SkIRect& r, SkIRect* sect = NULL) const;

    const SkBitmap& accessBitmap(bool changePixels);

    void eraseColor(SkColor eraseColor);

    _l8v(lockPixels, (), ());
    _l8v(unlockPixels, (), ());

    _l8v(setMatrixClip, (const SkMatrix& mat, const SkRegion& reg), (mat, reg));

    virtual void gainFocus(SkCanvas*) {}

    _l8v(drawPaint, (const SkDraw& drw, const SkPaint& paint), (drw, paint));
    _l8v(drawPoints, (const SkDraw& drw, SkCanvas::PointMode mode, size_t count, const SkPoint pts[], const SkPaint& paint), (drw, mode, count, pts, paint));
    _l8v(drawRect, (const SkDraw& drw, const SkRect& r, const SkPaint& paint), (drw, r, paint));
    _l8v(drawPath, (const SkDraw& drw, const SkPath& path, const SkPaint& paint), (drw, path, paint));
    _l8v(drawBitmap, (const SkDraw& drw, const SkBitmap& bitmap, const SkMatrix& matrix, const SkPaint& paint), (drw, bitmap, matrix, paint));
    _l8v(drawSprite, (const SkDraw& drw, const SkBitmap& bitmap, int x, int y, const SkPaint& paint), (drw, bitmap, x, y));
    _l8v(drawText, (const SkDraw& drw, const void* text, size_t len, SkScalar x, SkScalar y, const SkPaint& paint), (drw, text, len, x, y));
    _l8v(drawPosText, (const SkDraw& drw, const void* text, size_t len, const SkScalar pos[], SkScalar constY, int scalarsPerPos, const SkPaint& paint), (drw, text, len, pos, constY, scalarsPerPos, paint));
    _l8v(drawTextOnPath, (const SkDraw& drw, const void* text, size_t len, const SkPath& path, const SkMatrix* matrix, const SkPaint& paint), (drw, text, len, path, matrix, paint));
    _l9v(drawPosTextOnPath, (const SkDraw& draw, const void* text, size_t len, const SkPoint pos[], const SkPaint& paint, const SkPath& path, const SkMatrix* matrix), (draw, text, len, pos, paint, path, matrix));
    _l8v(drawVertices, (const SkDraw& drw, SkCanvas::VertexMode vmode, int vertexCount, const SkPoint verts[], const SkPoint texs[], const SkColor colors[], SkXfermode* xmode, const uint16_t indices[], int indexCount, const SkPaint& paint), (drw, vmode, vertexCount, verts, texs, colors, xmode, indices, indexCount, paint));
    _l8v(drawDevice, (const SkDraw& drw, SkDevice* dev, int x, int y, const SkPaint&), (drw, dev, x, y));

protected:
    _l8v(onAccessBitmap, (SkBitmap* bitmap), (bitmap));

private:
    SkBitmap fBitmap;
};

class SKCLASS(mySkDevice) : public SKCLASS(SkDevice)
{
public:
    mySkDevice(const SkBitmap& b) : SkDevice(b) {};
    virtual void drawText(const SkDraw& d, const void *text, size_t len, SkScalar x, SkScalar y, const SkPaint &paint) { ((mySkDraw *)(&d))->drawText((const char *)text, len, x, y, paint); };
};

