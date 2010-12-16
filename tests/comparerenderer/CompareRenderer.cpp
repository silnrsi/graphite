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
#include <cstdlib>
#include <cstdio>
#include <cstring>

#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef __GNUC__
#include <unistd.h>
#endif
#ifdef WIN32
#include <windows.h>
#endif

#include "RendererOptions.h"
#include "Renderer.h"
#include "RenderedLine.h"

#ifdef HAVE_GRAPHITE
#include "GrRenderer.h"
#endif

#include "GrNgRenderer.h"
#include "graphite2/XmlLog.h"

#ifdef HAVE_HARFBUZZNG
#include "HbNgRenderer.h"
#endif

#ifdef HAVE_USP10
#include "UniscribeRenderer.h"
#endif

#ifdef HAVE_ICU
#include "IcuRenderer.h"
#endif

const size_t NUM_RENDERERS = 3;

class CompareRenderer
{
public:
    CompareRenderer(const char * testFile, Renderer** renderers, bool verbose)
        : m_fileBuffer(NULL), m_numLines(0), m_lineOffsets(NULL),
        m_renderers(renderers), m_verbose(verbose), m_cfMask(ALL_DIFFERENCE_TYPES)
    {
        // read the file into memory for fast access
        struct stat fileStat;
        if (stat(testFile, &fileStat) == 0)
        {
            FILE * file = fopen(testFile, "r");
            if (file)
            {
                m_fileBuffer = new char[fileStat.st_size];
                if (m_fileBuffer)
                {
                    m_fileLength = fread(m_fileBuffer, 1, fileStat.st_size, file);
                    assert(m_fileLength == fileStat.st_size);
                    countLines();
                    findLines();
                    for (size_t r = 0; r < NUM_RENDERERS; r++)
                    {
                        if (m_renderers[r])
                        {
                            m_lineResults[r] = new RenderedLine[m_numLines];
                        }
                        else
                        {
                            m_lineResults[r] = NULL;
                        }
                        m_elapsedTime[r] = 0.0f;
                    }
                }
                fclose(file);
            }
            else
            {
                fprintf(stderr, "Error opening file %s\n", testFile);
            }
        }
        else
        {
            fprintf(stderr, "Error stating file %s\n", testFile);
            for (size_t r = 0; r < NUM_RENDERERS; r++)
            {
                m_lineResults[r] = NULL;
                m_elapsedTime[r] = 0.0f;
            }
        }
    }
    
    ~CompareRenderer()
    {
        delete m_fileBuffer;
        m_fileBuffer = NULL;
        for (size_t i = 0; i < NUM_RENDERERS; i++)
        {
            if (m_lineResults[i]) delete [] m_lineResults[i];
            m_lineResults[i] = NULL;
        }
        if (m_lineOffsets) delete m_lineOffsets;
        m_lineOffsets = NULL;
    }

    void runTests(FILE * log, int repeat = 1)
    {
        for (size_t r = 0; r < NUM_RENDERERS; r++)
        {
            if (m_renderers[r])
            {
                for (int i = 0; i < repeat; i++)
                    m_elapsedTime[r] += runRenderer(*m_renderers[r], m_lineResults[r]);
                fprintf(log, "Ran %s in %fs\n", m_renderers[r]->name(), m_elapsedTime[r]);
            }
        }
    }
    int compare(float tolerance, FILE * log)
    {
        int status = IDENTICAL;
        for (size_t i = 0; i < NUM_RENDERERS; i++)
        {
            for (size_t j = i + 1; j < NUM_RENDERERS; j++)
            {
                if (m_renderers[i] == NULL || m_renderers[j] == NULL) continue;
                if (m_lineResults[i] == NULL || m_lineResults[j] == NULL) continue;
                fprintf(log, "Comparing %s with %s\n", m_renderers[i]->name(), m_renderers[j]->name());
                for (size_t line = 0; line < m_numLines; line++)
                {
                    LineDifference ld = m_lineResults[i][line].compare(m_lineResults[j][line], tolerance);
                    ld = (LineDifference)(m_cfMask & ld);
                    if (ld)
                    {
                        fprintf(log, "Line %u %s\n", (unsigned int)line, DIFFERENCE_DESC[ld]);
                        for (size_t c = m_lineOffsets[line]; c < m_lineOffsets[line+1]; c++)
                        {
                            fprintf(log, "%c", m_fileBuffer[c]);
                        }
                        m_lineResults[i][line].dump(log);
                        fprintf(log, "%s\n", m_renderers[i]->name());
                        m_lineResults[j][line].dump(log);
                        fprintf(log, "%s\n", m_renderers[j]->name());
                        status |= ld;
                    }
                }
            }
        }
        return status;
    }
    void setDifferenceMask(LineDifference m) { m_cfMask = m; }
protected:
    float runRenderer(Renderer & renderer, RenderedLine * pLineResult)
    {
        unsigned int i = 0;
        const char * pLine = m_fileBuffer;
#ifdef __GNUC__
        struct timespec startTime;
        struct timespec endTime;
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &startTime);
#endif
#ifdef WIN32
        LARGE_INTEGER counterFreq;
        LARGE_INTEGER startCounter;
        LARGE_INTEGER endCounter;
        if (!QueryPerformanceFrequency(&counterFreq))
            fprintf(stderr, "Warning no high performance counter available\n");
        QueryPerformanceCounter(&startCounter);
#endif
        if (m_verbose)
        {
            while (i < m_numLines)
            {
                fprintf(stdout, "%s line %u\n", renderer.name(), i + 1);
                size_t lineLength = m_lineOffsets[i+1] - m_lineOffsets[i] - 1;
                pLine = m_fileBuffer + m_lineOffsets[i];
                renderer.renderText(pLine, lineLength, pLineResult + i);
                pLineResult[i].dump(stdout);
                ++i;
            }
        }
        else
        {
            while (i < m_numLines)
            {
                size_t lineLength = m_lineOffsets[i+1] - m_lineOffsets[i] - 1;
                pLine = m_fileBuffer + m_lineOffsets[i];
                renderer.renderText(pLine, lineLength, pLineResult + i);
                ++i;
            }
        }
#ifdef __GNUC__
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &endTime);
        long deltaSeconds = endTime.tv_sec - startTime.tv_sec;
        long deltaNs = endTime.tv_nsec - startTime.tv_nsec;
        if (deltaNs < 0)
        {
            deltaSeconds -= 1;
            deltaNs += 1000000000;
        }
        float elapsed = deltaSeconds + deltaNs / 1000000000.0f;
#endif
#ifdef WIN32
        QueryPerformanceCounter(&endCounter);
        float elapsed = (endCounter.QuadPart - startCounter.QuadPart) / static_cast<float>(counterFreq.QuadPart);
#endif
        return elapsed;
    }

    size_t countLines()
    {
        for (size_t i = 0; i < m_fileLength; i++)
        {
            if (m_fileBuffer[i] == '\n')
            {
                ++m_numLines;
            }
        }
        return m_numLines;
    }
    void findLines()
    {
        m_lineOffsets = new size_t[m_numLines+1];
        m_lineOffsets[0] = 0;
        int line = 0;
        for (size_t i = 0; i < m_fileLength; i++)
        {
            if (m_fileBuffer[i] == '\n')
            {
                m_lineOffsets[++line] = i + 1;
            }
            if (m_fileBuffer[i] > 0 && m_fileBuffer[i] < 32)
                m_fileBuffer[i] = 32;
        }
        m_lineOffsets[m_numLines] = m_fileLength;
    }
private:
    char * m_fileBuffer;
    size_t m_fileLength;
    size_t m_numLines;
    size_t * m_lineOffsets;
    Renderer** m_renderers;
    RenderedLine * m_lineResults[NUM_RENDERERS];
    float m_elapsedTime[NUM_RENDERERS];
    bool m_verbose;
    LineDifference m_cfMask;
};


int main(int argc, char ** argv)
{
    if (!parseOptions(argc, argv) ||
        !rendererOptions[OptFontFile].exists() ||
        !rendererOptions[OptTextFile].exists() ||
        !rendererOptions[OptSize].exists())
    {
        fprintf(stderr, "Usage:\n%s [options] -t utf8.txt -f font.ttf -s 12\n", argv[0]);
        fprintf(stderr, "Options:\n");
        showOptions();
        return -1;
    }

    const char * textFile = rendererOptions[OptTextFile].get(argv);
    const char * fontFile = rendererOptions[OptFontFile].get(argv);
    int fontSize = rendererOptions[OptSize].getInt(argv);
    FILE * log = stdout;
    if (rendererOptions[OptLogFile].exists())
    {
        log = fopen(rendererOptions[OptLogFile].get(argv), "wb");
        if (!log)
        {
            fprintf(stderr, "Failed to open log file %s\n",
                    rendererOptions[OptLogFile].get(argv));
            return -2;
        }
    }

    // TODO features

    Renderer* renderers[NUM_RENDERERS] = {NULL, NULL, NULL};
    int direction = (rendererOptions[OptRtl].exists())? 1 : 0;
    int segCacheSize = rendererOptions[OptSegCache].getInt(argv);
    
    {
        FILE * traceFile = fopen(rendererOptions[OptTrace].get(argv), "w");
        int logMask = (rendererOptions[OptLogMask].exists())? rendererOptions[OptLogMask].getInt(argv) :
            (gr2::GRLOG_SEGMENT | gr2::GRLOG_CACHE);
        gr2::graphite_start_logging(traceFile, static_cast<gr2::GrLogMask>(logMask));
    }

    if (rendererOptions[OptAlternativeFont].exists())
    {
        const char * altFontFile = rendererOptions[OptAlternativeFont].get(argv);
        if (rendererOptions[OptGraphiteNg].exists())
        {
            renderers[0] = (new GrNgRenderer(fontFile, fontSize, direction, segCacheSize));
            renderers[1] = (new GrNgRenderer(altFontFile, fontSize, direction, segCacheSize));
        }
#ifdef HAVE_GRAPHITE
        else if (rendererOptions[OptGraphite].exists())
        {
            renderers[0] = (new GrRenderer(fontFile, fontSize, direction));
            renderers[1] = (new GrRenderer(altFontFile, fontSize, direction));
        }
#endif
#ifdef HAVE_HARFBUZZNG
        else if (rendererOptions[OptHarfbuzzNg].exists())
        {
            renderers[0] = (new HbNgRenderer(fontFile, fontSize, direction));
            renderers[1] = (new HbNgRenderer(altFontFile, fontSize, direction));
        }
#endif
#ifdef HAVE_ICU
        else if (rendererOptions[OptIcu].exists())
        {
            renderers[0] = (new IcuRenderer(fontFile, fontSize, direction));
            renderers[1] = (new IcuRenderer(altFontFile, fontSize, direction));
        }
#endif
    }
    else
    {
#ifdef HAVE_GRAPHITE
        if (rendererOptions[OptGraphite].exists())
            renderers[0] = (new GrRenderer(fontFile, fontSize, direction));
#endif
        if (rendererOptions[OptGraphiteNg].exists())
            renderers[1] = (new GrNgRenderer(fontFile, fontSize, direction, segCacheSize));
#ifdef HAVE_HARFBUZZNG
        if (rendererOptions[OptHarfbuzzNg].exists())
            renderers[2] = (new HbNgRenderer(fontFile, fontSize, direction));
#endif
#ifdef HAVE_ICU
        if (rendererOptions[OptIcu].exists())
            renderers[2] = (new IcuRenderer(fontFile, fontSize, direction));
#endif
    }

    if (renderers[0] == NULL && renderers[1] == NULL && renderers[2] == NULL)
    {
        fprintf(stderr, "Please specify at least 1 renderer\n");
        showOptions();
        return -3;
    }   

    CompareRenderer compareRenderers(textFile, renderers, rendererOptions[OptVerbose].exists());
    if (rendererOptions[OptRepeat].exists())
        compareRenderers.runTests(log, rendererOptions[OptRepeat].getInt(argv));
    else
        compareRenderers.runTests(log);
    // set compare options
    if (rendererOptions[OptIgnoreGlyphIdDifferences].exists())
    {
        compareRenderers.setDifferenceMask((LineDifference)(ALL_DIFFERENCE_TYPES ^ DIFFERENT_GLYPHS));
    }
    int status = 0;
    if (rendererOptions[OptCompare].exists())
        status = compareRenderers.compare(rendererOptions[OptTolerance].getFloat(argv), log);

    for (size_t i = 0; i < NUM_RENDERERS; i++)
    {
        if (renderers[i])
        {
            delete renderers[i];
            renderers[i] = NULL;
        }
    }
    if (rendererOptions[OptLogFile].exists()) fclose(log);
    if (rendererOptions[OptTrace].exists()) gr2::graphite_stop_logging();

    return status;
}
