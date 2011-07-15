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

#include "inject.h"
#include <jni.h>
#include <stdio.h>
#include "SkTypeface.h"
#include "SkStream.h"
#include <utils/AssetManager.h>
#include <android_runtime/android_util_AssetManager.h>

extern func_map thismap;


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


extern "C" void Java_com_sil_mjph_helloworld1_HelloWorld1_injectJNI( JNIEnv* env, jobject thiz )
{
    inject_fns("libinject-graphite.so", "libskia.so", &thismap, 12);
}

myfontmap *myfonts = NULL;
static FT_Library gFTLibrary = NULL;

void *gettable(const void *recp, unsigned int tag, size_t *len)
{
    myfontmap *rec = (myfontmap *)recp;
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

extern "C" jobject Java_com_sil_mjph_helloworld1_HelloWorld1_addFontResourceJNI( JNIEnv *env, jobject thiz, jobject jassetMgr, jstring jpath, jstring jname )
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
    myfontmap *f = new myfontmap;
    f->next = myfonts;
    f->tf = tf;
    f->name = name;
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
    f->grface = gr_make_face(f, (gr_get_table_fn)&gettable, 0);
    myfonts = f;
//    return (int)(void *)(f->tf);
    return res;
}


