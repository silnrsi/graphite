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
*/
#include <cstdio>
#include <graphite2/Segment.h>
#include <graphite2/Log.h>
#include "inc/Main.h"
#include "inc/Silf.h"
#include "inc/Face.h"
#include "inc/CachedFace.h"
#include "inc/Segment.h"
#include "inc/SegCache.h"
#include "inc/SegCacheStore.h"
#include "inc/UtfCodec.h"
#include "inc/TtfTypes.h"
#include "inc/TtfUtil.h"

using namespace graphite2;

inline gr_face * api_cast(CachedFace *p) { return static_cast<gr_face*>(static_cast<Face*>(p)); }

template <typename utf_itr>
void resolve_unicode_to_glyphs(const Face & face, utf_itr first, size_t n_chars, uint16 * glyphs)
{
    Face::Table  cmap_tbl = Face::Table(face, "cmap");
	const void * cmap = TtfUtil::FindCmapSubtable(cmap_tbl, 3, 1);

	for (; n_chars; --n_chars, ++first)
	{
		const uint32 usv = *first;
		assert(usv < 0xFFFF); 	// only lower plane supported for this test
		*glyphs++ = TtfUtil::CmapSubtable4Lookup(cmap, usv);
	}
}

bool checkEntries(CachedFace
 * face, const char * testString, uint16 * glyphString, size_t testLength)
{
    gr_feature_val * defaultFeatures = gr_face_featureval_for_lang(api_cast(face), 0);
    SegCache * segCache = face->cacheStore()->getOrCreate(0, *defaultFeatures);
    const SegCacheEntry * entry = segCache->find(glyphString, testLength);
    if (!entry)
    {
        unsigned int offset = 0;
        const char * space = strstr(testString + offset, " ");
        if (space)
        {
            while (space)
            {
                unsigned int wordLength = (space - testString) - offset;
                if (wordLength)
                {
                    entry = segCache->find(glyphString + offset, wordLength);
                    if (!entry)
                    {
                        fprintf(stderr, "failed to find substring at offset %u length %u in '%s'\n",
                                offset, wordLength, testString);
                        return false;
                    }
                }
                while (offset < (space - testString) + 1u)
                {
                    ++offset;
                }
                while (testString[offset] == ' ')
                {
                    ++offset;
                }
                space = strstr(testString + offset, " ");
            }
            if (offset < testLength)
            {
                entry = segCache->find(glyphString + offset, testLength - offset);
                if (!entry)
                {
                    fprintf(stderr, "failed to find last word at offset %u in '%s'\n",
                            offset, testString);
                    return false;
                }
            }
        }
        else
        {
            fprintf(stderr, "entry not found for '%s'\n", testString);
            return false;
        }
    }
    gr_featureval_destroy(defaultFeatures);
    return true;
}

bool testSeg(CachedFace
* face, const gr_font *sizedFont,
             const char * testString,
             size_t * testLength, uint16 ** testGlyphString)
{
    const void * badUtf8 = NULL;
    *testLength = gr_count_unicode_characters(gr_utf8, testString,
                                                    testString + strlen(testString),
                                                    &badUtf8);
    *testGlyphString = gralloc<uint16>(*testLength + 1);
    resolve_unicode_to_glyphs(*face, utf8::iterator(testString), *testLength, *testGlyphString);

    gr_segment * segA = gr_make_seg(sizedFont, api_cast(face), 0, NULL, gr_utf8, testString,
                        *testLength, 0);
    assert(segA);
    if ((gr_seg_n_slots(segA) == 0) ||
        !checkEntries(face, testString, *testGlyphString, *testLength))
        return false;
   return true;
}

int main(int argc, char ** argv)
{
    assert(sizeof(uintptr) == sizeof(void*));
    const char * fileName = NULL;
    if (argc > 1)
    {
        fileName = argv[1];
    }
    else
    {
        fprintf(stderr, "Usage: %s font.ttf\n", argv[0]);
        return 1;
    }
    CachedFace *face = static_cast<CachedFace *>(static_cast<Face *>(
        gr_make_file_face_with_seg_cache(fileName, 10, gr_face_default)));
    if (!face)
    {
        fprintf(stderr, "Invalid font, failed to parse tables\n");
        return 3;
    }
    gr_start_logging(api_cast(face), "grsegcache.json");
    gr_font *sizedFont = gr_make_font(12, api_cast(face));
    const char * testStrings[] = { "a", "aa", "aaa", "aaab", "aaac", "a b c",
        "aaa ", " aa", "aaaf", "aaad", "aaaa"};
    uint16 * testGlyphStrings[sizeof(testStrings)/sizeof(char*)];
    size_t testLengths[sizeof(testStrings)/sizeof(char*)];
    const size_t numTestStrings = sizeof(testStrings) / sizeof (char*);
    
    for (size_t i = 0; i < numTestStrings; i++)
    {
        testSeg(face, sizedFont, testStrings[i], &(testLengths[i]), &(testGlyphStrings[i]));
    }
    gr_feature_val * defaultFeatures = gr_face_featureval_for_lang(api_cast(face), 0);
    SegCache * segCache = face->cacheStore()->getOrCreate(0, *defaultFeatures);
    unsigned int segCount = segCache->segmentCount();
    long long accessCount = segCache->totalAccessCount();
    if (segCount != 10 || accessCount != 16)
    {
        fprintf(stderr, "SegCache contains %u entries, which were used %lld times\n",
            segCount, accessCount);
        return -2;
    }
    for (size_t i = 0; i < numTestStrings; i++)
    {
        if (!checkEntries(face, testStrings[i], testGlyphStrings[i], testLengths[i]))
            return -3;
    }
    segCount = segCache->segmentCount();
    accessCount = segCache->totalAccessCount();
    if (segCount != 10 || accessCount != 29)
    {
        fprintf(stderr, "SegCache after repeat contains %u entries, which were used %lld times\n",
            segCount, accessCount);
        return -2;
    }
    // test purge
    size_t len = 0;
    uint16 * testGlyphString = NULL;
    testSeg(face, sizedFont, "ba", &len, &testGlyphString);
    segCount = segCache->segmentCount();
    accessCount = segCache->totalAccessCount();
    if (segCount > 10 || accessCount != 30)
    {
        fprintf(stderr, "SegCache after purge contains %u entries, which were used %lld times\n",
            segCount, accessCount);
        return -2;
    }
    gr_font_destroy(sizedFont);
    gr_face_destroy(api_cast(face));
    gr_featureval_destroy(defaultFeatures);

    gr_stop_logging(api_cast(face));
    return 0;
}
