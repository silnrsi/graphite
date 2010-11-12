#pragma once

#include <new>
#include "Renderer.h"
#include <graphiteng/Types.h>
#include <graphiteng/GrFace.h>
#include <graphiteng/GrFont.h>
#include <graphiteng/GrSegment.h>
#include <graphiteng/Slot.h>

namespace gr2 = org::sil::graphite::v2;

class GrNgRenderer : public Renderer
{
public:
    GrNgRenderer(const char * fontFile, int fontSize, int textDir)
        : m_rtl(textDir),
        m_grFace(make_file_face(fontFile, gr2::ePreload)),
        m_grFont(0)
    {
        if (m_grFace)
            m_grFont = gr2::make_font(static_cast<float>(fontSize), m_grFace);
    }
    virtual ~GrNgRenderer()
    {
        gr2::destroy_font(m_grFont);
        gr2::destroy_face(m_grFace);
        m_grFont = NULL;
        m_grFace = NULL;
    }
    virtual void renderText(const char * utf8, size_t length, RenderedLine * result)
    {
        const void * pError = NULL;
        if (!m_grFace) return;
        size_t numCodePoints = gr2::count_unicode_characters(gr2::kutf8,
            reinterpret_cast<const void*>(utf8), reinterpret_cast<const void*>(utf8 + length), &pError);
        if (pError)
            fprintf(stderr, "Invalid Unicode pos %d\n", reinterpret_cast<const char*>(pError) - utf8);
        gr2::GrSegment* pSeg = make_GrSegment(m_grFont, m_grFace, 0u, gr2::kutf8, utf8, numCodePoints, m_rtl);
        if (!pSeg) return;
        RenderedLine * renderedLine = new(result) RenderedLine(gr2::number_of_slots_in_segment(pSeg), advance_X(pSeg));
        int i = 0;
        for (const gr2::Slot* s = first_slot_in_segment(pSeg); s; s = next_slot_in_segment(s), ++i)
            (*renderedLine)[i].set(gid(s), origin_X(s), origin_Y(s), before(s), after(s));
        
//         for (int i = 0; i < seg.length(); i++)
//         {
//             (*renderedLine)[i].set(seg[i].gid(), seg[i].originX(), seg[i].originY(), seg[i].before(), seg[i].after());
//         }
        gr2::destroy_GrSegment(pSeg);
    }
    virtual const char * name() const { return "graphiteng"; }
private:
    int m_rtl;
    gr2::GrFace * m_grFace;
    gr2::GrFont * m_grFont;
};
