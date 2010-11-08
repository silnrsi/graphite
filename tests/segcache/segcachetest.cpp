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
#include "Main.h"
#include "Silf.h"
#include "GrFaceImp.h"
#include "GrSegmentImp.h"
#include "SegCache.h"
#include "processUTF.h"
#include "TtfTypes.h"
#include "TtfUtil.h"
#include <boost/config/posix_features.hpp>


namespace gr2 = org::sil::graphite::v2;


class CmapProcessor
{
public:
    CmapProcessor(gr2::GrFace * face, uint16 * buffer) :
        m_cmapTable(TtfUtil::FindCmapSubtable(face->getTable(tagCmap, NULL), 3, 1)),
        m_buffer(buffer), m_pos(0) {};
    bool processChar(uint32 cid)      //return value indicates if should stop processing
    {
        assert(cid < 0xFFFF); // only lower plane supported for this test
        m_buffer[m_pos++] = TtfUtil::Cmap31Lookup(m_cmapTable, cid);
        return true;
    }
    size_t charsProcessed() const { return m_pos; } //number of characters processed. Usually starts from 0 and incremented by processChar(). Passed in to LIMIT::needMoreChars
private:
    void * m_cmapTable;
    uint16 * m_buffer;
    size_t m_pos;
};

bool checkEntries(const GrFace * face, const char * testString, uint16 * glyphString, size_t testLength)
{
    const SegCacheEntry * entry = face->silf(0)->segmentCache()->find(glyphString, testLength);
    if (!entry)
    {
        size_t offset = 0;
        const char * space = strstr(testString + offset, " ");
        if (space)
        {
            while (space)
            {
                size_t wordLength = (space - testString) - offset;
                if (wordLength)
                {
                    entry = face->silf(0)->segmentCache()->find(glyphString + offset, wordLength);
                    if (!entry)
                    {
                        fprintf(stderr, "failed to find substring at offset %lu length %lu in '%s'\n",
                                offset, wordLength, testString);
                        return false;
                    }
                }
                while (offset < (space - testString) + 1)
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
                entry = face->silf(0)->segmentCache()->find(glyphString + offset, testLength - offset);
                if (!entry)
                {
                    fprintf(stderr, "failed to find last word at offset %lu in '%s'\n",
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
    return true;
}


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
    FILE * log = fopen("grsegcache.xml", "w");
    gr2::startGraphiteLogging(log, GRLOG_SEGMENT);
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
    uint16 * testGlyphStrings[sizeof(testStrings)/sizeof(char*)];
    size_t testLengths[sizeof(testStrings)/sizeof(char*)];
    const size_t numTestStrings = sizeof(testStrings) / sizeof (char*);
    
    for (size_t i = 0; i < numTestStrings; i++)
    {
        size_t testLength = count_unicode_characters_to_nul(gr2::kutf8, testStrings[i], &badUtf8);
        testGlyphStrings[i] = gr2::gralloc<gr2::uint16>(testLength + 1);
        CharacterCountLimit limit(gr2::kutf8, testStrings[i], testLength);
        CmapProcessor cmapProcessor(face, testGlyphStrings[i]);
        IgnoreErrors ignoreErrors;
        testLengths[i] = testLength;
        processUTF(limit, &cmapProcessor, &ignoreErrors);

        gr2::GrSegment * segA = gr2::make_GrSegment(sizedFont, face, 0, gr2::kutf8, testStrings[i],
                            testLength, 0);
        assert(segA);
        if (!checkEntries(face, testStrings[i], testGlyphStrings[i], testLengths[i]))
            return -1;
    }
    size_t segCount = face->silf(0)->segmentCache()->segmentCount();
    long long accessCount = face->silf(0)->segmentCache()->totalAccessCount();
    if (segCount != 7 || accessCount != 13)
    {
        fprintf(stderr, "SegCache contains %lu entries, which were used %Ld times\n",
            segCount, accessCount);
        return -2;
    }
    for (size_t i = 0; i < numTestStrings; i++)
    {
        if (!checkEntries(face, testStrings[i], testGlyphStrings[i], testLengths[i]))
            return -3;
    }
    segCount = face->silf(0)->segmentCache()->segmentCount();
    accessCount = face->silf(0)->segmentCache()->totalAccessCount();
    if (segCount != 7 || accessCount != 23)
    {
        fprintf(stderr, "SegCache after repeat contains %lu entries, which were used %Ld times\n",
            segCount, accessCount);
        return -2;
    }
    destroy_GrFont(sizedFont);
    destroy_GrFace(face);
    destroy_file_face_handle(fileface);
    
    gr2::stopGraphiteLogging();
    return 0;
}
