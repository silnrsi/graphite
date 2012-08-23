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

#include <new>
#include <memory>
#include <string>
#include "Renderer.h"
#include "FeatureParser.h"
#include <graphite2/Types.h>
#include <graphite2/Segment.h>
#include <graphite2/Log.h>

class Gr2Face : private std::auto_ptr<gr_face>
{
	bool m_cached;

public:
	Gr2Face(const char * fontFile, int cache, const std::string & logPath)
    :  std::auto_ptr<gr_face>(cache == 0
          ? gr_make_file_face(fontFile, gr_face_preloadGlyphs)
          : gr_make_file_face_with_seg_cache(fontFile, cache, gr_face_cacheCmap | gr_face_preloadGlyphs)),
       m_cached(cache > 0)
	{
        if (!get()) return;

    	if (logPath.size())	gr_start_logging(get(), logPath.c_str());
	}

	~Gr2Face() throw()
	{
    	gr_stop_logging(get());
        gr_face_destroy(get());
        release();
	}

	bool cached() const 		{ return m_cached; }
	operator bool () const		{ return get() != 0; }
	operator gr_face* () const	{ return get(); }
};


class Gr2Renderer : public Renderer
{
public:
    Gr2Renderer(Gr2Face & face, int fontSize, int textDir, FeatureParser * features)
	: m_rtl(textDir),
	  m_grFace(face),
	  m_grFont(0),
	  m_grFeatures(0)
    {
        if (m_grFace)
        {
            m_grFont = gr_make_font(static_cast<float>(fontSize), m_grFace);
            if (features)
            {
                m_grFeatures = gr_face_featureval_for_lang(m_grFace, features->langId());
                for (size_t i = 0; i < features->featureCount(); i++)
                {
                    const gr_feature_ref * ref = gr_face_find_fref(m_grFace, features->featureId(i));
                    if (ref)
                        gr_fref_set_feature_value(ref, features->featureSValue(i), m_grFeatures);
                }
            }
            else
            {
                m_grFeatures = gr_face_featureval_for_lang(m_grFace, 0);
            }
        }
        m_name = !m_grFace.cached() ? "graphite2 (uncached)" : "graphite2 (cached)";
    }
    virtual ~Gr2Renderer()
    {
        gr_featureval_destroy(m_grFeatures);
        gr_font_destroy(m_grFont);
    }

    virtual void renderText(const char * utf8, size_t length, RenderedLine * result, FILE *log)
    {
        const void * pError = NULL;
        if (!m_grFace)
        {
            new(result) RenderedLine();
            return;
        }
        size_t numCodePoints = gr_count_unicode_characters(gr_utf8,
            reinterpret_cast<const void*>(utf8), reinterpret_cast<const void*>(utf8 + length), &pError);
        if (pError)
            fprintf(stderr, "Invalid Unicode pos %d\n", static_cast<int>(reinterpret_cast<const char*>(pError) - utf8));
        gr_segment* pSeg = gr_make_seg(m_grFont, m_grFace, 0u, m_grFeatures, gr_utf8, utf8, numCodePoints, m_rtl ? 1 : 0);
        if (!pSeg)
        {
            fprintf(stderr, "Failed to create segment\n");
            new(result) RenderedLine(0, .0f);
            return;
        }
        RenderedLine * renderedLine = new(result) RenderedLine(gr_seg_n_slots(pSeg),
                                                               gr_seg_advance_X(pSeg));
        int i = 0;
        for (const gr_slot* s = gr_seg_first_slot(pSeg); s;
             s = gr_slot_next_in_segment(s), ++i)
            (*renderedLine)[i].set(gr_slot_gid(s), gr_slot_origin_X(s),
                                   gr_slot_origin_Y(s), gr_slot_before(s),
                                   gr_slot_after(s));
        gr_seg_destroy(pSeg);
    }
    virtual const char * name() const { return m_name; }
private:
    int m_rtl;
    Gr2Face   m_grFace;
    gr_font * m_grFont;
    gr_feature_val * m_grFeatures;
    const char * m_name;
};
