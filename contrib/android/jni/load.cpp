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

#include <dlfcn.h>
#include "_linker.h"
#include <linux/elf.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include "load.h"

//typedef void (SkDraw::*drawTextFn)(const char *, size_t, SkScalar, SkScalar, const SkPaint&) const;

static pthread_mutex_t dl_lock = PTHREAD_MUTEX_INITIALIZER;

//drawTextFn pDrawText = &SkDraw::drawText;
void reloc_fns(soinfo *si, Elf32_Rel *rel, unsigned count, func_map *map, unsigned num_map);
#ifdef ANDROID_SH_LINKER
void reloca_fns(soinfo *si, Elf32_Rela *rela, unsigned count, func_map *map, unsigned num_map);
#endif

void load_fns(const char *srcname, const char *targetname, func_map *map, int num_map)
{
    soinfo *soHead = (soinfo *)dlopen("libdl.so", 0);
    soinfo *soTarget = (soinfo *)dlopen(targetname, 0);
    soinfo *soSrc = (soinfo *)dlopen(srcname, 0);
    soinfo *si, *sislast, *sitlast;
    int i, j;

    // turn names into pointers
    for (i = 0; i < num_map; ++i)
    {
//  See "ELF for the Arm Architecture" sect. 4.6.3, p 21. But we don't strip because that's Thumb code's job
        map[i].ptarget = dlsym(soTarget, map[i].starget);
        map[i].psrc = dlsym(soSrc, map[i].ssrc);
//        map[i].ptarget = (void *)((size_t)dlsym(soTarget, map[i].starget) & ~1);
//        map[i].psrc = (void *)((size_t)dlsym(soSrc, map[i].ssrc) & ~1);
    }
    
    pthread_mutex_lock(&dl_lock);
    // fixup referencing libraries
    for (si = soHead; si; si = si->next)
    {
        unsigned *d;
        if (si->next == soSrc)
            sislast = si;
        else if (si->next == soTarget)
            sitlast = si;
        // don't redirect ourselves, that could cause nasty loops
        if (si == soSrc)
            continue;
        if (si->dynamic)
        {
            for (d = si->dynamic; *d; d +=2)
            {
                if (d[0] == DT_NEEDED && (void *)d[1] == soTarget)
                {
                    ((void **)d)[1] = soSrc;
                    if (si->plt_rel)
                        reloc_fns(si, si->plt_rel, si->plt_rel_count, map, num_map);
                    if (si->rel)
                        reloc_fns(si, si->rel, si->rel_count, map, num_map);
    #ifdef ANDROID_SH_LINKER
                    if (si->plt_rela)
                        reloca_fns(si, si->plt_rela, si->plt_rela_count, map, num_map);
                    if (si->rela)
                        reloca_fns(si, si->rela, si->rela_count, map, num_map);
    #endif
                    soSrc->refcount++;
                    if (soTarget->refcount > 0) soTarget->refcount--;
                    break;
                }
            }
        }
    }

// The following code doesn't work and causes crashes in some situations
// Also we regain control of the JNI calls again.
#if 0
    Elf32_Sym *symSrc;
    // move our library to the front and swap names
    goto notdone;
    sitlast->next = soSrc;
    si = soSrc->next;  // as temp var
    soSrc->next = soTarget->next;
    sislast->next = soTarget;
    soTarget->next = si;
    notdone:
    strncpy((char *)soSrc->name, soTarget->name, 128);
    strncpy((char *)soTarget->name, srcname, 128);

    // copy and modify the target's symbol table
    symSrc = (Elf32_Sym *)malloc(soTarget->nchain * 16);
    memcpy(symSrc, soTarget->symtab, soTarget->nchain * 16);
    for (i = 0; i < soTarget->nchain; i++)
    {
        if ((symSrc[i].st_info & 0xF) != STT_FUNC) continue;

        for (j = 0; j < num_map; j++)
        {
            if ((void *)(symSrc[i].st_value + soTarget->base) == map[j].ptarget)
            {
                symSrc[i].st_value = (Elf32_Addr)map[j].psrc - soTarget->base;
                break;
            }
        }
        symSrc[i].st_value = symSrc[i].st_value + soTarget->base - soSrc->base;
    }
    soSrc->symtab = symSrc;
    // copy string and hash tables
    soSrc->strtab = soTarget->strtab;
    soSrc->nbucket = soTarget->nbucket;
    soSrc->nchain = soTarget->nchain;
    soSrc->bucket = soTarget->bucket;
    soSrc->chain = soTarget->chain;
    pthread_mutex_unlock(&dl_lock);
    // all done
    done:
#endif
    dlclose(soHead);
    dlclose(soTarget);
    dlclose(soSrc);
}

void unload_loaded(char *srcname, char *tgtname, func_map *map, int num_map)
{
    soinfo *soSrc = (soinfo *)dlopen(srcname, 0);
    soinfo *soTgt = (soinfo *)dlopen(tgtname, 0);
    unsigned *d;

    strncpy((char *)soSrc->name, srcname, 128);
    strncpy((char *)soTgt->name, tgtname, 128);
    free(soSrc->symtab);
    
    for (d = soSrc->dynamic; *d; d +=2)
    {
        if (d[0] == DT_SYMTAB)
            soSrc->symtab = (Elf32_Sym *)d[1];
        else if (d[0] == DT_STRTAB)
            soSrc->strtab = (const char *)d[1];
        else if (d[0] == DT_HASH)
        {
            soSrc->nbucket = ((unsigned *)(soSrc->base + d[1]))[0];
            soSrc->nchain = ((unsigned *)(soSrc->base + d[1]))[1];
            soSrc->bucket = ((unsigned **)(soSrc->base + d[1]))[2];
            soSrc->chain = ((unsigned **)(soSrc->base + d[1]))[3];
        }
    }

    dlclose(soSrc);
    dlclose(soTgt);
}

void reloc_fns(soinfo *si, Elf32_Rel *rel, unsigned count, func_map *map, unsigned num_map)
{
    unsigned idx, imap;
    Elf32_Sym *symtab = si->symtab;
    const char *strtab = si->strtab;

    for (idx = 0; idx < count; ++idx, ++rel)
    {
        unsigned type = ELF32_R_TYPE(rel->r_info);
        unsigned sym = ELF32_R_SYM(rel->r_info);
        unsigned roffset = (unsigned)(rel->r_offset + si->base);

        switch(type)
        {
#if defined(ANDROID_ARM_LINKER)
        case R_ARM_JUMP_SLOT :
        case R_ARM_GLOB_DAT :
        case R_ARM_ABS32 :
#elif defined(ANDROID_X86_LINKER)
        case R_386_JUMP_SLOT :
        case R_386_GLOB_DAT :
#endif
            for (imap = 0; imap < num_map; imap++)
            {
                if (!strcmp(strtab + symtab[sym].st_name, map[imap].starget))
                    *(void **)roffset = map[imap].psrc;
            }
                break;
        default :
            break;
        }
    }
}

#ifdef ANDROID_SH_LINKER
void reloca_fns(soinfo *si, Elf32_Rela *rela, unsigned count, func_map *map, unsigned num_map)
{
    unsigned idx, imap;

    for (idx = 0; idx < count; ++idx, ++rel)
    {
        unsigned type = ELF32_R_TYPE(rela->info);
        unsigned sym = ELF32_R_SYM(rela->info);
        unsigned roffset = (unsigned)(rela->reloc + si->base);

        switch(type)
        {
#if defined(ANDROID_ARM_LINKER)
        case R_ARM_JUMP_SLOT :
        case R_ARM_GLOB_DAT :
#elif defined(ANDROID_X86_LINKER)
        case R_386_JUMP_SLOT :
        case R_386_GLOB_DAT :
#endif
            for (imap = 0; imap < num_map; imap++)
            {
                if (*(void **)roffset == map[imap].ptarget)
                    *(void **)roffset = map[imap].psrc;
            }
                break;
        default :
            break;
        }
    }
}
#endif


