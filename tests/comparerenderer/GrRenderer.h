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
#include "Renderer.h"
#include "FeatureParser.h"

#include <graphite/GrClient.h>
#include <graphite/Segment.h>
#include <graphite/FileFont.h>
#include <graphite/ITextSource.h>

#include "GrUtfTextSrc.h"

class GrRenderer : public Renderer
{
public:
    GrRenderer(const char * fontFile, int fontSize, int direction, FeatureParser * features)
        : m_fileFont(fontFile, static_cast<float>(fontSize), 72, 72), m_features(features)
        
    {  
        m_layout.setStartOfLine(false);
        m_layout.setEndOfLine(false);
        m_layout.setDumbFallback(true);
        m_layout.setJustifier(NULL);
        m_layout.setRightToLeft(direction);
    }
    void renderText(const char * utf8, size_t length, RenderedLine * result)
    {
        if ((length == 0) || !m_fileFont.isValid())
            new(result) RenderedLine();
        GrUtfTextSrc textSrc;
        textSrc.setFeatures(m_features);
        textSrc.setText(reinterpret_cast<gr::utf8 *>(const_cast<char*>(utf8)), length);
        try
        {
            gr::RangeSegment seg(&m_fileFont, &textSrc, &m_layout, 0, length);
            std::pair<gr::GlyphIterator,gr::GlyphIterator> glyphs = seg.glyphs();
            int numGlyphs = glyphs.second - glyphs.first;
            RenderedLine * renderedLine = new(result) RenderedLine(numGlyphs, seg.advanceWidth());
            int i = 0;
//            if (m_layout.rightToLeft())
//                i = numGlyphs - 1;
            gr::GlyphIterator iGlyph = glyphs.first;
            while (iGlyph != glyphs.second)
            {
                gr::GlyphInfo & gi = *iGlyph;
                (*renderedLine)[i].set(gi.glyphID(), gi.origin(), gi.yOffset(), gi.firstChar(), gi.lastChar());
                ++iGlyph;
//                if (m_layout.rightToLeft())
//                    --i;
//                else
                    ++i;
            }
        }
        catch (...)
        {
            fprintf(stderr, "Exception in Graphite\n");
            new(result) RenderedLine();
        }
    }
    virtual const char * name() const { return "graphite"; }
private:
    gr::FileFont m_fileFont;
    gr::LayoutEnvironment m_layout;
    FeatureParser * m_features;
};

