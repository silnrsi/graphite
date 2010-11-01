#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H
#include <hb-shape.h>
#include <hb-buffer.h>
#include <hb-font.h>
#include <hb-ft.h>
#include <hb-glib.h>
#include <hb-ot.h>
#include <hb-ot-layout.h>
#include <glib/gunicode.h>


#include "Renderer.h"

void hbngFtDestroy(void *user_data)
{
    // the face is destroyed at the same time as the HB face, so shouldn't be needed
}

class HbNgRenderer : public Renderer
{
public:
    HbNgRenderer(const char * fileName, int fontSize, int textDir)
        : m_ftLibrary(NULL), m_ftFace(NULL),
        m_face(NULL), m_font(NULL), m_bufferLength(1024)
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
        m_buffer = hb_buffer_create(m_bufferLength);
    }
    virtual ~HbNgRenderer()
    {
        if (m_buffer) hb_buffer_destroy(m_buffer);
        if (m_font) hb_font_destroy(m_font);
        if (m_face) hb_face_destroy(m_face);
        if (m_ftFace) FT_Done_Face(m_ftFace);
        if (m_ftLibrary) FT_Done_FreeType(m_ftLibrary);
    }
    virtual void renderText(const char * utf8, size_t length, RenderedLine * result)
    {
        if (length > m_bufferLength)
        {
            hb_buffer_destroy(m_buffer);
            m_bufferLength = length;
            hb_buffer_create(m_bufferLength);
        }
        hb_buffer_clear(m_buffer);
        hb_buffer_add_utf8(m_buffer, utf8, length, 0, length);
        hb_unicode_funcs_t * unicodeFuncs = hb_glib_get_unicode_funcs();
        hb_script_t script = hb_unicode_get_script(unicodeFuncs, g_utf8_get_char(utf8));
        hb_buffer_set_script(m_buffer, script);
        hb_language_t lang = hb_ot_tag_to_language(HB_OT_TAG_DEFAULT_LANGUAGE);
        hb_buffer_set_language(m_buffer, lang);
        hb_shape(m_font, m_face, m_buffer, NULL, 0);
        hb_glyph_info_t * infos = hb_buffer_get_glyph_infos(m_buffer);
        hb_glyph_position_t * positions = hb_buffer_get_glyph_positions(m_buffer);
        size_t numGlyphs = hb_buffer_get_length(m_buffer);
        RenderedLine * renderedLine = new(result) RenderedLine(numGlyphs);
        int dx = 0;
        for (size_t i = 0; i < numGlyphs; i++)
        {
            // Note cluster numbers are not really same as before/after positions
            (*renderedLine)[i].set(infos[i].codepoint, (dx + positions[i].x_offset)/64.0f,
                                   positions[i].y_offset/64.0f,
                                   infos[i].cluster, infos[i].cluster);
            dx += positions[i].x_advance;
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
    size_t m_bufferLength;
};
