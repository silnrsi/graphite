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
#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_TRUETYPE_TABLES_H

#include <harfbuzz-shaper.h>
#include <harfbuzz-impl.h>
#include <harfbuzz-global.h>
#include <harfbuzz-gpos.h>

#include "Renderer.h"
#include "FeatureParser.h"

static HB_UChar32 getChar(const HB_UChar16 *string, hb_uint32 length, hb_uint32 &i)
{
    HB_UChar32 ch;
    if (HB_IsHighSurrogate(string[i])
        && i < length - 1
        && HB_IsLowSurrogate(string[i + 1])) {
        ch = HB_SurrogateToUcs4(string[i], string[i + 1]);
        ++i;
    } else {
        ch = string[i];
    }
    return ch;
}

static HB_Bool hb_stringToGlyphs(HB_Font font, const HB_UChar16 *string, hb_uint32 length, HB_Glyph *glyphs, hb_uint32 *numGlyphs, HB_Bool /*rightToLeft*/)
{
    FT_Face face = (FT_Face)font->userData;
    if (length > *numGlyphs)
        return false;

    int glyph_pos = 0;
    for (hb_uint32 i = 0; i < length; ++i) {
        glyphs[glyph_pos] = FT_Get_Char_Index(face, getChar(string, length, i));
        ++glyph_pos;
    }

    *numGlyphs = glyph_pos;

    return true;
}

static void hb_getAdvances(HB_Font /*font*/, const HB_Glyph * /*glyphs*/, hb_uint32 numGlyphs, HB_Fixed *advances, int /*flags*/)
{
    for (hb_uint32 i = 0; i < numGlyphs; ++i)
        advances[i] = 0; // ### not tested right now
}

static HB_Bool hb_canRender(HB_Font font, const HB_UChar16 *string, hb_uint32 length)
{
    FT_Face face = (FT_Face)font->userData;

    for (hb_uint32 i = 0; i < length; ++i)
        if (!FT_Get_Char_Index(face, getChar(string, length, i)))
            return false;

    return true;
}

static HB_Error hb_getSFntTable(void *font, HB_Tag tableTag, HB_Byte *buffer, HB_UInt *length)
{
    FT_Face face = (FT_Face)font;
    FT_ULong ftlen = *length;
    FT_Error error = 0;

    if (!FT_IS_SFNT(face))
        return HB_Err_Invalid_Argument;

    error = FT_Load_Sfnt_Table(face, tableTag, 0, buffer, &ftlen);
    *length = ftlen;
    return (HB_Error)error;
}

HB_Error hb_getPointInOutline(HB_Font font, HB_Glyph glyph, int flags, hb_uint32 point, HB_Fixed *xpos, HB_Fixed *ypos, hb_uint32 *nPoints)
{
    HB_Error error = HB_Err_Ok;
    FT_Face face = (FT_Face)font->userData;

    int load_flags = (flags & HB_ShaperFlag_UseDesignMetrics) ? FT_LOAD_NO_HINTING : FT_LOAD_DEFAULT;

    if ((error = (HB_Error)FT_Load_Glyph(face, glyph, load_flags)))
        return error;

    if (face->glyph->format != ft_glyph_format_outline)
        return (HB_Error)HB_Err_Invalid_SubTable;

    *nPoints = face->glyph->outline.n_points;
    if (!(*nPoints))
        return HB_Err_Ok;

    if (point > *nPoints)
        return (HB_Error)HB_Err_Invalid_SubTable;

    *xpos = face->glyph->outline.points[point].x;
    *ypos = face->glyph->outline.points[point].y;

    return HB_Err_Ok;
}

void hb_getGlyphMetrics(HB_Font, HB_Glyph, HB_GlyphMetrics *metrics)
{
    // ###
    metrics->x = metrics->y = metrics->width = metrics->height = metrics->xOffset = metrics->yOffset = 0;
}

HB_Fixed hb_getFontMetric(HB_Font, HB_FontMetric )
{
    return 0; // ####
}

const HB_FontClass hb_fontClass = {
    hb_stringToGlyphs, hb_getAdvances, hb_canRender,
    hb_getPointInOutline, hb_getGlyphMetrics, hb_getFontMetric
};

class HbRenderer : public Renderer
{
public:
    HbRenderer(const char * fileName, int fontSize, int textDir)
        : m_ftLibrary(NULL), m_ftFace(NULL)
    {

        if ((FT_Init_FreeType(&m_ftLibrary) == 0) &&
            (FT_New_Face(m_ftLibrary, fileName, 0, &m_ftFace) == 0))
        {
            FT_Size_RequestRec request;
            request.type = FT_SIZE_REQUEST_TYPE_NOMINAL;
            // scale by 64 to avoid low values failing, scaled back again in renderText
            request.width = request.height = fontSize << 6;
            request.horiResolution = request.vertResolution = 72;// above is 26.6
            FT_Request_Size(m_ftFace, &request);
            m_face = HB_NewFace(m_ftFace, hb_getSFntTable);
            m_font.klass = &hb_fontClass;
            m_font.userData = m_ftFace;
            m_font.x_ppem = m_ftFace->size->metrics.x_ppem;
            m_font.y_ppem = m_ftFace->size->metrics.y_ppem;
            m_font.x_scale = m_ftFace->size->metrics.x_scale;
            m_font.y_scale = m_ftFace->size->metrics.y_scale;

            m_shaper.string = m_buffer;
            m_shaper.kerning_applied = false;
            m_shaper.item.bidiLevel = textDir;
            m_shaper.font = &m_font;
            m_shaper.face = m_face;
            m_shaper.glyphIndicesPresent = false;
            m_shaper.glyphs = NULL;
            m_shaper.attributes = NULL;
            m_shaper.advances = NULL;
            m_shaper.offsets = NULL;
            m_shaper.log_clusters = NULL;
        }
    }
    virtual ~HbRenderer()
    {
        if (m_ftFace) FT_Done_Face(m_ftFace);
        if (m_ftLibrary) FT_Done_FreeType(m_ftLibrary);
    }

#define FREE(x) if(x) {free(x); x=NULL; }

    virtual void renderText(const char * utf8, size_t length, RenderedLine * result, FILE *log)
    {
using graphite2 {
        graphite2::ToUtf16Processor processor(m_buffer, m_bufferLength * 2);
        graphite2::IgnoreErrors ingore;
        graphite2::BufferLimit bufferLimit(gr_utf8, reinterpret_cast<const void *>(utf8), reinterpret_cast<const void *>(utf8 + length));
        graphite2::processUTF<graphite2::BufferLimit, graphite2::ToUtf16Processor, graphite2::IgnoreErrors>(bufferLimit, &processor, &ignore);
        m_shaper.stringLength = processor.uint16Processed;
        m_buffer[m_shaper.stringLength] = 0;
        m_shaper.script = HB_Script_Common;
        m_shaper.num_glyphs = 2 * m_shaper.stringLength;
        size_t numglyphs = 0;
        while (m_shaper.num_glyphs > numglyphs)
        {
            numglyphs = m_shaper.num_glyphs;
            FREE(m_shaper.glyphs);
            FREE(m_shaper.attributes);
            FREE(m_shaper.advances);
            FREE(m_shaper.offsets);
            FREE(m_shaper.log_clusters);
            m_shaper.glyphs = (HB_Glyph *)malloc(numglyphs * sizeof(HB_Glyph));
            m_shaper.attributes = (HB_GlyphAttributes *)malloc(numglyphs * sizeof(HB_GlyphAttributes));
            m_shaper.advances = (HB_Fixed *)malloc(numglyphs * sizeof(HB_Fixed));
            m_shaper.offsets = (HB_FixedPoint *)malloc(numglyphs * sizeof(HB_FixedPoint));
            m_shaper.log_clusters = (unsigned short *)malloc(numglyphs * sizeof(short));
            HB_ShapeItem(&m_shaper);
        }

        RenderedLine * renderedLine = new(result) RenderedLine(m_shaper.num_glyphs);
        float dx = 0., dy = 0.;
        for (size_t i = 0; i < m_shaper.num_glyphs; i++)
        {
            // Note cluster numbers are not really same as before/after positions
            (*renderedLine)[i].set(m_shaper.glyphs[i], (dx + m_shaper.offsets[i].x)/64.0f,
                                   (dy + m_shaper.offsets[i].y)/64.0f,
                                   m_shaper.log_clusters[i], m_shaper.log_clusters[i]);
            dx += m_shaper.advances[i];
//            dy += positions[i].y_advance;
        }
        renderedLine->setAdvance(dx/64.0f);
        FREE(m_shaper.glyphs);
        FREE(m_shaper.attributes);
        FREE(m_shaper.advances);
        FREE(m_shaper.offsets);
        FREE(m_shaper.log_clusters);
}
    }
    virtual const char * name() const { return "harfbuzz"; }
private:
    FT_Library m_ftLibrary;
    FT_Face m_ftFace;
    HB_Face m_face;
    HB_FontRec m_font;
    HB_ShaperItem m_shaper;
    unsigned short m_buffer[1024];
};
