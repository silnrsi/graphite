#include <cstdlib>
#include <cstdio>
#include <cstring>

#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "Renderer.h"
#include "GrRenderer.h"
#include "RenderedLine.h"

const size_t NUM_RENDERERS = 3;
const char * RENDER_NAMES[3] = { "gr", "grng", "hbng" };

class CompareRenderer
{
public:
    CompareRenderer(const char * testFile, Renderer** renderers)
        : m_fileBuffer(NULL), m_numLines(0), m_renderers(renderers)
    {
        // read the file into memory for fast access
        struct stat fileStat;
        if (stat(testFile, &fileStat))
        {
            FILE * file = fopen(testFile, "r");
            if (file)
            {
                m_fileBuffer = new char[fileStat.st_size];
                if (m_fileBuffer)
                {
                    size_t m_fileLength = fread(m_fileBuffer, 1, fileStat.st_size, file);
                    assert(m_fileLength == fileStat.st_size);
                    countLines();
                    for (size_t r = 0; r < NUM_RENDERERS; r++)
                    {
                        if (m_renderers[r])
                        {
                            m_lineResults[r] = new RenderedLine[m_numLines];
                            m_elapsedTime[r] = 0.0f;
                        }
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
        }
    }
    
    ~CompareRenderer()
    {
        delete m_fileBuffer;
        m_fileBuffer = NULL;
        for (size_t i = 0; i < NUM_RENDERERS; i++)
        {
            delete m_lineResults[i];
            m_lineResults[i] = NULL;
        }
    }

    void runTests()
    {
        for (size_t r = 0; r < NUM_RENDERERS; r++)
        {
            if (m_renderers[r])
            {
                m_elapsedTime[r] += runRenderer(*m_renderers[r], m_lineResults[r]);
            }
            fprintf(stdout, "Ran %s in %fs\n", RENDER_NAMES[r], m_elapsedTime[r]);
        }
    }
    void compare()
    {
        for (size_t i = 0; i < NUM_RENDERERS; i++)
        {
            for (size_t j = i + 1; j < NUM_RENDERERS; j++)
            {
                if (m_lineResults[i] == NULL || m_lineResults[j] == NULL) continue;
                fprintf(stdout, "Comparing %s with %s\n", RENDER_NAMES[i], RENDER_NAMES[j]);
                for (size_t line = 0; line < m_numLines; line++)
                {
                    LineDifference ld = m_lineResults[i][line].compare(m_lineResults[j][line]);
                    if (ld)
                    {
                        fprintf(stdout, "Line %d %s\n", DIFFERENCE_DESC[ld]);
                    }
                }
            }
        }
    }
protected:
    float runRenderer(Renderer & renderer, RenderedLine * pLineResult)
    {
        size_t i = 0;
        const char * pLine = m_fileBuffer;
        const char * pLineBreak = m_fileBuffer;
        struct timespec startTime;
        struct timespec endTime;
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &startTime);
        while (i < m_numLines)
        {
            size_t lineLength;
            if (i + 1 == m_numLines)
                lineLength = m_fileLength - (pLine - m_fileBuffer);
            else
            {
                while (*pLineBreak != '\n') ++pLineBreak;
                lineLength = pLineBreak - pLine;
            }
            renderer.renderText(pLine, lineLength, pLineResult + i);
            ++pLineBreak;
            pLine = pLineBreak + 1;
        }
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &endTime);
        long deltaSeconds = endTime.tv_sec - startTime.tv_sec;
        long deltaNs = endTime.tv_nsec - startTime.tv_nsec;
        if (deltaNs < 0)
        {
            deltaSeconds -= 1;
            deltaNs += 1000000000;
        }
        float elapsed = deltaSeconds + deltaNs / 1000000000.0f;
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
private:
    char * m_fileBuffer;
    size_t m_fileLength;
    size_t m_numLines;
    Renderer** m_renderers;
    RenderedLine * m_lineResults[NUM_RENDERERS];
    float m_elapsedTime[NUM_RENDERERS];
};


void help(const char * app)
{
    fprintf(stderr, "Usage: %s textfile font size [gr] [grng] [hb]\n", app); 
}

int main(int argc, char ** argv)
{
    if (argc < 4)
    {
        help(argv[0]);
        return -1;
    }
    const char * textFile = argv[1];
    const char * fontFile = argv[2];
    int fontSize = atoi(argv[3]);

    Renderer* renderers[NUM_RENDERERS] = {NULL, NULL, NULL};
    int direction = 0; // ltr
    for (int i = 4; i < argc; i++)
    {
//        if (strcmp(argv[i], "gr") == 0)
//            renderers.push_back(new GrRenderer(fontFile, fontSize, direction));
//        else if (strcmp(argv[i], "grng") == 0)
//            renderers.push_back(new GrNgRenderer(fontFile, fontSize, direction));
//        else if (strcmp(argv[i], "hb") == 0)
//            renderers.push_back(new HbNgRenderer(fontFile, fontSize, direction));
    }
    CompareRenderer compareRenderers(textFile, renderers);
    compareRenderers.runTests();
    compareRenderers.compare();
    return 0;
}
