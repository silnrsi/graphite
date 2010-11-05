/*  GRAPHITENG LICENSING

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
*/
#include <graphiteng/GrFace.h>
#include <graphiteng/GrFont.h>
#include <graphiteng/GrSegment.h>
#include "Silf.h"
#include "GrFaceImp.h"
#include "GrSegmentImp.h"
#include "SegCache.h"
#include <boost/config/posix_features.hpp>


namespace gr2 = org::sil::graphite::v2;

int main(int argc, char ** argv)
{

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
    gr2::FileFaceHandle *fileface;
    if (!(fileface = gr2::make_file_face_handle(fileName)))
    {
        fprintf(stderr, "Invalid font, failed to read tables\n");
        return 2;
    }

    gr2::GrFace *face = gr2::make_GrFace_from_file_face_handle(fileface, gr2::ePreload);
    if (!face)
    {
        fprintf(stderr, "Invalid font, failed to parse tables\n");
        return 3;
    }
    gr2::enable_segment_cache(face, 4096, 0);
    gr2::GrFont *sizedFont = gr2::make_GrFont(12, face);
    const void * badUtf8 = NULL;
    const char * testStrings[] = { "a", "aa", "aaa", "aaaa", "aaab", "a b c", "aaa ", " aa" };

    for (size_t i = 0; i < sizeof(testStrings) / sizeof (char*); i++)
    {
        size_t testAlength = count_unicode_characters_to_nul(gr2::kutf8, testStrings[i], &badUtf8);
        gr2::GrSegment * segA = gr2::make_GrSegment(sizedFont, face, 0, gr2::kutf8, testStrings[i],
                            testAlength, 0);
        assert(segA);
        SegCacheEntry * entry = face->silf(0)->segmentCache()->find(segA->first(), testAlength);
        if (!entry)
        {
            size_t offset = 0;
            const char * space = strstr(testStrings[i] + offset, " ");
            const Slot * slot = segA->first();
            if (space)
            {
                while (space)
                {
                    size_t wordLength = (space - testStrings[i]) - offset;
                    if (wordLength)
                    {
                        entry = face->silf(0)->segmentCache()->find(slot, wordLength);
                        if (!entry)
                        {
                            fprintf(stderr, "failed to find substring at offset %d length %d in '%s'\n",
                                    offset, wordLength, testStrings[i]);
                            return -1;
                        }
                    }
                    while (offset < (space - testStrings[i]) + 1)
                    {
                        slot = slot->next();
                        ++offset;
                    }
                    while (testStrings[i][offset] == ' ')
                    {
                        slot = slot->next();
                        ++offset;
                    }
                    space = strstr(testStrings[i] + offset, " ");
                }
                if (offset < testAlength)
                {
                    entry = face->silf(0)->segmentCache()->find(slot, testAlength - offset);
                    if (!entry)
                    {
                        fprintf(stderr, "failed to find last word at offset %d in '%s'\n", offset, testStrings[i]);
                        return -1;
                    }
                }
            }
            else
            {
                fprintf(stderr, "entry not found for '%s'\n", testStrings[i]);
                return -1;
            }
        }
    }
    size_t segCount = face->silf(0)->segmentCache()->segmentCount();
    long long accessCount = face->silf(0)->segmentCache()->totalAccessCount();
    if (segCount != 7 || accessCount != 13)
    {
        fprintf(stderr, "SegCache contains %d entries, which were used %d times\n",
            segCount, accessCount);
        return -2;
    }
    return 0;
}
