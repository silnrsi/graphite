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
        : m_fileFace(gr2::make_file_face_handle(fontFile)),
        m_rtl(textDir),
        m_grFace(make_GrFace_from_file_face_handle(m_fileFace, gr2::ePreload)),
        m_grFont(0)
    {
        if (m_grFace)
            m_grFont = gr2::make_GrFont(static_cast<float>(fontSize), m_grFace);
    }
    virtual ~GrNgRenderer()
    {
        gr2::destroy_GrFont(m_grFont);
        gr2::destroy_GrFace(m_grFace);
        m_grFont = NULL;
        m_grFace = NULL;
        gr2::destroy_file_face_handle(m_fileFace);
        m_fileFace = NULL;
    }
    virtual void renderText(const char * utf8, size_t length, RenderedLine * result)
    {
        const void * pError = NULL;
        if (!m_grFace) return;
        size_t numCodePoints = gr2::count_unicode_characters(gr2::kutf8,
            reinterpret_cast<const void*>(utf8), reinterpret_cast<const void*>(utf8 + length), &pError);
        if (pError)
            fprintf(stderr, "Invalid Unicode pos %ld\n", reinterpret_cast<const char*>(pError) - utf8);
        gr2::GrSegment* pSeg = make_GrSegment(m_grFont, m_grFace, 0u, gr2::kutf8, utf8, numCodePoints, m_rtl);
        if (!pSeg) return;
        RenderedLine * renderedLine = new(result) RenderedLine(gr2::length(pSeg), advance_X(pSeg));
        int i = 0;
        for (gr2::SlotHandle s = first(pSeg); !s.isNull(); s = s.next(), ++i)
            (*renderedLine)[i].set(s.gid(), s.originX(), s.originY(), s.before(), s.after());
        
//         for (int i = 0; i < seg.length(); i++)
//         {
//             (*renderedLine)[i].set(seg[i].gid(), seg[i].originX(), seg[i].originY(), seg[i].before(), seg[i].after());
//         }
        gr2::destroy_GrSegment(pSeg);
    }
    virtual const char * name() const { return "graphiteng"; }
private:
    int m_rtl;
    gr2::FileFaceHandle * m_fileFace;
    gr2::GrFace * m_grFace;
    gr2::GrFont * m_grFont;
};
