#include "Renderer.h"

#include <graphite/GrClient.h>
#include <graphite/Segment.h>
#include <graphite/FileFont.h>
#include <graphite/ITextSource.h>

#include "GrUtfTextSrc.h"

class GrRenderer : public Renderer
{
public:
    GrRenderer(const char * fontFile, int fontSize, int direction)
        : m_fileFont(fontFile, fontSize, 72, 72)
        
    {  
        m_layout.setStartOfLine(false);
        m_layout.setEndOfLine(false);
        m_layout.setDumbFallback(true);
        m_layout.setJustifier(NULL);
        m_layout.setRightToLeft(direction);
    }
    void renderText(const char * utf8, size_t length, RenderedLine * result)
    {
        GrUtfTextSrc textSrc;
        textSrc.setText(reinterpret_cast<gr::utf8 *>(const_cast<char*>(utf8)), length);
        gr::RangeSegment seg(&m_fileFont, &textSrc, &m_layout, 0, length);
        std::pair<gr::GlyphIterator,gr::GlyphIterator> glyphs = seg.glyphs();
        int numGlyphs = glyphs.second - glyphs.first;
        RenderedLine * renderedLine = new(result) RenderedLine(numGlyphs, seg.advanceWidth());
        int i = 0;
        gr::GlyphIterator iGlyph = glyphs.first;
        while (iGlyph != glyphs.second)
        {
            gr::GlyphInfo & gi = *iGlyph;
            (*renderedLine)[i].set(gi.glyphID(), gi.origin(), gi.yOffset(), gi.firstChar(), gi.lastChar());
            ++iGlyph;
            ++i;
        }
    }
    virtual const char * name() const { return "graphite"; }
private:
    gr::FileFont m_fileFont;
    gr::LayoutEnvironment m_layout;
};

