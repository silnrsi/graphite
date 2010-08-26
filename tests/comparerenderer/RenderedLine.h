#pragma once

#include <cassert>

class RenderedLine;

typedef enum {
    IDENTICAL = 0,
    MORE_GLYPHS = 1,
    LESS_GLYPHS = 2,
    DIFFERENT_ADVANCE = 4,
    DIFFERENT_GLYPHS = 8,
    DIFFERENT_POSITIONS = 16,
    ALL_DIFFERENCE_TYPES = MORE_GLYPHS | LESS_GLYPHS | DIFFERENT_ADVANCE | DIFFERENT_GLYPHS | DIFFERENT_POSITIONS
} LineDifference;

const char * DIFFERENCE_DESC[] = {
    "same",//0
    "more glyphs",//1
    "less glyphs",//2
    "",
    "different advance",//4
    "","","",
    "different glyphs",//8
    "","","","","","","",
    "different positions"//16
};

class GlyphInfo
{
    public:
        GlyphInfo()
        : m_gid(0), m_x(0), m_y(0), m_firstChar(0), m_lastChar(0)
        {}
        GlyphInfo(size_t theGid, float xPos, float yPos, size_t first, size_t last)
            : m_gid(theGid), m_x(xPos), m_y(yPos), m_firstChar(first), m_lastChar(last)
        {}
        void set(size_t theGid, float xPos, float yPos, size_t first, size_t last)
        {
            m_gid = theGid;
            m_x = xPos;
            m_y = yPos;
            m_firstChar = first;
            m_lastChar = last;
        }
        float x() const { return m_x; }
        float y() const { return m_y; }
        size_t glyph() const { return m_gid; }
        LineDifference compare(GlyphInfo & cf, float tolerance)
        {
            if (m_gid != cf.m_gid) return DIFFERENT_GLYPHS;
            // do we need a tolerance here?
            if (m_x > cf.m_x + tolerance || m_x < cf.m_x - tolerance ||
                m_y > cf.m_y + tolerance || m_y < cf.m_y - tolerance)
            {
                return DIFFERENT_POSITIONS;
            }
            return IDENTICAL;
        }
        void dump(FILE * f)
        {
            fprintf(f, "[%3u,%6.2f,%5.2f,%2u,%2u]", (unsigned int)m_gid,
                    m_x, m_y, (unsigned int)m_firstChar, (unsigned int)m_lastChar);
        }
    private:
        size_t m_gid;
        float m_x;
        float m_y;
        size_t m_firstChar;
        size_t m_lastChar;
};


class RenderedLine
{
    public:
        RenderedLine()
        : m_numGlyphs(0), m_advance(0), m_glyphs(NULL)
        {}
        RenderedLine(size_t numGlyphs, float adv = 0.0f)
        : m_numGlyphs(numGlyphs), m_advance(adv), m_glyphs(new GlyphInfo[numGlyphs])
        {
            
        }
        ~RenderedLine()
        {
            delete [] m_glyphs;
            m_glyphs = NULL;
        }
        void setAdvance(float newAdv) { m_advance = newAdv; }
        void dump(FILE * f)
        {
            for (size_t i = 0; i < m_numGlyphs; i++)
            {
                fprintf(f, "%2u", (unsigned int)i);
                (*this)[i].dump(f);
                // only 3 glyphs fit on 80 char line
                if ((i + 1) % 3 == 0) fprintf(f, "\n");
            }
            fprintf(f, "(%2u,%4.3f)", (unsigned int)m_numGlyphs, m_advance);
        }
        LineDifference compare(RenderedLine & cf, float tolerance)
        {
            if (m_numGlyphs > cf.m_numGlyphs) return MORE_GLYPHS;
            if (m_numGlyphs < cf.m_numGlyphs) return LESS_GLYPHS;
            if (m_advance > cf.m_advance + tolerance ||
                m_advance < cf.m_advance - tolerance) return DIFFERENT_ADVANCE;
            for (size_t i = 0; i < m_numGlyphs; i++)
            {
                LineDifference ld = (*this)[i].compare(cf[i], tolerance);
                if (ld) return ld;
            }
            return IDENTICAL;
        }
        GlyphInfo & operator [] (size_t i) { assert(i < m_numGlyphs); return m_glyphs[i]; }
        const GlyphInfo & operator [] (size_t i) const { assert(i < m_numGlyphs); return m_glyphs[i]; }
        // define placement new for windows
        void * operator new (size_t size, RenderedLine * p) { return p; }
        void operator delete (void *, RenderedLine * p) { }
    private:
        size_t m_numGlyphs;
        float m_advance;
        GlyphInfo * m_glyphs;
};

