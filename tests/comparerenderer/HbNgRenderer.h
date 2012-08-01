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
#include <hb.h>
#include <hb-ot.h>
#include <hb-ft.h>
#include <hb-glib.h>
/*
#include <hb-buffer.h>
#include <hb-font.h>
#include <hb-ot-layout.h>
*/
#include <glib/gunicode.h>

#include "Renderer.h"
#include "FeatureParser.h"

static const char *shapers[] = {
    "ot", NULL
};

void hbngFtDestroy(void *user_data)
{
    // the face is destroyed at the same time as the HB face, so shouldn't be needed
}

class HbNgRenderer : public Renderer
{
public:
    HbNgRenderer(const char * fileName, int fontSize, int textDir, FeatureParser * features)
        : m_ftLibrary(NULL), m_ftFace(NULL),
        m_face(NULL), m_font(NULL), m_feats(NULL), m_featCount(0)
    {
        if ((FT_Init_FreeType(&m_ftLibrary) == 0) &&
            (FT_New_Face(m_ftLibrary, fileName, 0, &m_ftFace) == 0))
        {
            if (FT_Select_Charmap(m_ftFace, FT_ENCODING_UNICODE))
                fprintf(stderr, "Failed to find unicode charmap\n");
            FT_Size_RequestRec request;
            request.type = FT_SIZE_REQUEST_TYPE_NOMINAL;
            // scale by 64 to avoid low values failing, scaled back again in renderText
            request.width = request.height = fontSize << 6;
            request.horiResolution = request.vertResolution = 72;// above is 26.6
            if (FT_Request_Size(m_ftFace, &request) == 0)
            {
                //m_face = hb_ft_face_create(m_ftFace, hbngFtDestroy);
                m_face = hb_ft_face_create_cached(m_ftFace);
                m_font = hb_ft_font_create(m_ftFace, hbngFtDestroy);
                //m_font = hb_ft_font_create_cache_glyphmetrics(m_ftFace, hbngFtDestroy);
            }
            else
            {
                fprintf(stderr, "FT_Request_Size %d failed\n", fontSize);
            }
        }
        m_buffer = hb_buffer_create();
        if (features)
        {
            m_featCount = features->featureCount() + 1;
            m_feats = new hb_feature_t[m_featCount];
            if (m_feats)
            {
                m_feats[0].tag = HB_TAG(' ', 'R', 'N', 'D');
                m_feats[0].value = 0;
                m_feats[0].start = 0;
                m_feats[0].end = -1;
                for (size_t i = 1; i < m_featCount; i++)
                for (size_t i = 0; i < m_featCount; i++)
                {
                    m_feats[i].tag = features->featureId(i-1);
                    m_feats[i].value = features->featureUValue(i-1);
                    m_feats[i].start = 0;
                    m_feats[i].end = -1;
                }
            }
            else
            {
                m_featCount = 0;
            }
        }
    }
    virtual ~HbNgRenderer()
    {
        if (m_buffer) hb_buffer_destroy(m_buffer);
        if (m_font) hb_font_destroy(m_font);
        if (m_face) hb_face_destroy(m_face);
        if (m_ftFace) FT_Done_Face(m_ftFace);
        if (m_ftLibrary) FT_Done_FreeType(m_ftLibrary);
        delete m_feats;
    }
    virtual void renderText(const char * utf8, size_t length, RenderedLine * result, FILE *log)
    {
//        if (length > m_bufferLength)
//        {
//            hb_buffer_destroy(m_buffer);
//            m_bufferLength = length;
//            hb_buffer_create(m_bufferLength);
//        }
        hb_buffer_reset(m_buffer);
        hb_buffer_add_utf8(m_buffer, utf8, length, 0, length);
        hb_unicode_funcs_t * unicodeFuncs = hb_glib_get_unicode_funcs();
        hb_script_t script = hb_buffer_get_script(m_buffer);
        hb_buffer_set_script(m_buffer, script);
        hb_language_t lang = hb_ot_tag_to_language(HB_OT_TAG_DEFAULT_LANGUAGE);
        hb_buffer_set_language(m_buffer, lang);
        //hb_feature_t feats = {HB_TAG(' ', 'R', 'N', 'D'), 0, 0, -1};
        hb_shape_full(m_font, m_buffer, m_feats, m_featCount, shapers);
        hb_glyph_info_t * infos = hb_buffer_get_glyph_infos(m_buffer, NULL);
        hb_glyph_position_t * positions = hb_buffer_get_glyph_positions(m_buffer, NULL);
        size_t numGlyphs = hb_buffer_get_length(m_buffer);
        RenderedLine * renderedLine = new(result) RenderedLine(numGlyphs);
        float dx = 0., dy = 0.;
        for (size_t i = 0; i < numGlyphs; i++)
        {
            // Note cluster numbers are not really same as before/after positions
            (*renderedLine)[i].set(infos[i].codepoint, (dx + positions[i].x_offset)/64.0f,
                                   (dy + positions[i].y_offset)/64.0f,
                                   infos[i].cluster, infos[i].cluster);
            dx += positions[i].x_advance;
            dy += positions[i].y_advance;
        }
        renderedLine->setAdvance(dx/64.0f);
    }
    virtual const char * name() const { return "harfbuzzng"; }
private:
    FT_Library m_ftLibrary;
    FT_Face m_ftFace;
    hb_face_t * m_face;
    hb_font_t * m_font;
    hb_buffer_t * m_buffer;
    hb_feature_t* m_feats;
    size_t m_featCount;
};
