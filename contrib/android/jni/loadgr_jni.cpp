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
    If not, write to the Free Software Foundation, 51 Franklin Street, 
    Suite 500, Boston, MA 02110-1335, USA or visit their web page on the 
    internet at http://www.fsf.org/licenses/lgpl.html.

    Alternatively, you may use this library under the terms of the Mozilla
    Public License (http://mozilla.org/MPL) or under the GNU General Public
    License, as published by the Free Sofware Foundation; either version
    2 of the license or (at your option) any later version.
*/

#include "load.h"
#include "load_gr.h"
#include <jni.h>
#include <stdio.h>
#include "SkStream.h"
#include <utils/AssetManager.h>
#include <android_runtime/android_util_AssetManager.h>
#include "ft2build.h"
#include FT_FREETYPE_H
#include FT_TRUETYPE_TABLES_H

typedef struct fnmap func_map;
extern func_map thismap;

typedef struct rec_ft_table {
    unsigned long tag;
    size_t len;
    void *buffer;
    struct rec_ft_table *next;

    ~rec_ft_table() { free(buffer); delete next; }
} rec_ft_table;

typedef struct fontmap {
    struct fontmap *next;
    const char *name;
    SkTypeface *tf;
    FT_Face ftface;
    rec_ft_table *tables;
    gr_face *grface;
    int rtl;
} fontmap;

fontmap *myfonts = NULL;

class AssetStream : public SkStream {
public:
    AssetStream(android::Asset* asset, bool hasMemoryBase) : fAsset(asset)
    {
        fMemoryBase = hasMemoryBase ? fAsset->getBuffer(false) : NULL;
    }

    virtual ~AssetStream()
    {
        delete fAsset;
    }
    
    virtual const void* getMemoryBase()
    {
        return fMemoryBase;
    }

    virtual bool rewind()
    {
        off_t pos = fAsset->seek(0, SEEK_SET);
        return pos != (off_t)-1;
    }
    
    virtual size_t read(void* buffer, size_t size)
    {
        ssize_t amount;
        
        if (NULL == buffer)
        {
            if (0 == size)  // caller is asking us for our total length
                return fAsset->getLength();
            
            // asset->seek returns new total offset
            // we want to return amount that was skipped
            
            off_t oldOffset = fAsset->seek(0, SEEK_CUR);
            if (-1 == oldOffset)
                return 0;
            off_t newOffset = fAsset->seek(size, SEEK_CUR);
            if (-1 == newOffset)
                return 0;
            
            amount = newOffset - oldOffset;
        }
        else
        {
            amount = fAsset->read(buffer, size);
        }
        
        if (amount < 0)
            amount = 0;
        return amount;
    }
    
private:
    android::Asset*      fAsset;
    const void* fMemoryBase;
};


extern "C" void Java_org_sil_palaso_Graphite_loadGraphite( JNIEnv* env, jobject thiz )
{
    load_fns("libload-graphite.so", "libskia.so", &thismap, 12);
}

static FT_Library gFTLibrary = NULL;

void *gettable(const void *recp, unsigned int tag, size_t *len)
{
    fontmap *rec = (fontmap *)recp;
    rec_ft_table *r, *rlast;
    for (r = rec->tables, rlast = NULL; r; rlast = r, r = r->next)
    {
        if (r->tag == tag)
        {
            if (len) *len = r->len;
            return r->buffer;
        }
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
    if (r->buffer)
    	FT_Load_Sfnt_Table(rec->ftface, tag, 0, (FT_Byte *)(r->buffer), &length);
    if (len) *len = length;
    r->len = length;
    return r->buffer;
}

extern "C" jobject Java_org_sil_palaso_Graphite_addFontResource( JNIEnv *env, jobject thiz, jobject jassetMgr, jstring jpath, jstring jname, jint rtl )
{
    android::AssetManager* mgr = android::assetManagerForJavaObject(env, jassetMgr);
    if (NULL == mgr) return 0;
    
    const char *str = env->GetStringUTFChars(jpath, NULL);
    android::Asset* asset = mgr->open(str, android::Asset::ACCESS_BUFFER);
    env->ReleaseStringUTFChars(jpath, str);
    if (NULL == asset) return 0;
    
    AssetStream *aStream = new AssetStream(asset, true);
    SkTypeface * tf = SkTypeface::CreateFromStream(aStream);
    if (NULL == tf) return 0;
    jclass c = env->FindClass("android/graphics/Typeface");
    jmethodID cid = env->GetMethodID(c, "<init>", "(I)V");
    jobject res = env->NewObject(c, cid, (int)(void *)(tf));

    const char * name = env->GetStringUTFChars(jname, NULL);     // leaky
    fontmap *f = new fontmap;
    f->next = myfonts;
    f->tf = tf;
    f->name = rtl ? "" : name;
    f->rtl = rtl ? 3 : 0;
    if (!gFTLibrary && FT_Init_FreeType(&gFTLibrary))
    {
        delete f->tf;
        delete f;
        return 0;
    }
    FT_Open_Args    args;
    args.flags = FT_OPEN_MEMORY;
    args.memory_base = (FT_Byte *)(aStream->getMemoryBase());
    args.memory_size = aStream->read(NULL, 0);
    FT_Error err = FT_Open_Face(gFTLibrary, &args, 0, &f->ftface);
    aStream->unref();
    if (err)
    {
        delete f->tf;
        delete f;
        return 0;
    }
    f->tables = NULL;
    f->grface = gr_make_face(f, (gr_get_table_fn)&gettable, gr_face_preloadAll);
    delete f->tables;
    myfonts = f;
    if (rtl)
    {
        SkTypeface *tfw = SkTypeface::CreateFromStream(aStream);
        if (tfw)
        {
            fontmap *fw = new fontmap;
            fw->next = myfonts;
            fw->tf = tfw;
            fw->name = name;
            fw->rtl = 1;
            fw->ftface = f->ftface;
            fw->tables = NULL;
            fw->grface = f->grface;
            myfonts = fw;
        }
    }
//    return (int)(void *)(f->tf);
    return res;
}

extern "C" gr_face *gr_face_from_tf(SkTypeface *tf, int *rtl)
{
    fontmap *f;
    for (f = myfonts; f; f = f->next)
        if (f->tf == tf)
        {
            if (rtl) *rtl = f->rtl;
            return f->grface;
        }
    return 0;
}

extern "C" SkTypeface *tf_from_name(const char *name)
{
    fontmap *f;
    for (f = myfonts; f; f = f->next)
        if (!strcmp(f->name, name)) return f->tf;
    return 0;
}

