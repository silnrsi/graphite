#pragma once

#include <new>
#include "Renderer.h"
#include <graphiteng/Types.h>
#include <graphiteng/face.h>
#include <graphiteng/font.h>
#include <graphiteng/SegmentHandle.h>
#include <graphiteng/SlotHandle.h>

namespace gr2 = org::sil::graphite::v2;

class GrNgRenderer : public Renderer
{
public:
    GrNgRenderer(const char * fontFile, int fontSize, int textDir)
        : m_fileFace(gr2::make_file_face_handle(fontFile)),
        m_rtl(textDir),
        m_grFace(make_GrFace_from_file_face_handle(m_fileFace, gr2::ePreload)),
        m_grFont(gr2::make_GrFont(static_cast<float>(fontSize), m_grFace))
    {

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
        size_t numCodePoints = gr2::SegmentHandle::countUnicodeCharacters(gr2::SegmentHandle::kutf8,
            reinterpret_cast<const void*>(utf8), reinterpret_cast<const void*>(utf8 + length), &pError);
        if (pError)
            fprintf(stderr, "Invalid Unicode pos %ld\n", reinterpret_cast<const char*>(pError) - utf8);
        gr2::SegmentHandle seg(m_grFont, m_grFace, 0u, gr2::SegmentHandle::kutf8, utf8, numCodePoints, m_rtl);
        RenderedLine * renderedLine = new(result) RenderedLine(seg.length(), seg.advanceX());
        int i = 0;
        for (gr2::SlotHandle s = seg.first(); !s.isNull(); s = s.next(), ++i)
            (*renderedLine)[i].set(s.gid(), s.originX(), s.originY(), s.before(), s.after());
        
//         for (int i = 0; i < seg.length(); i++)
//         {
//             (*renderedLine)[i].set(seg[i].gid(), seg[i].originX(), seg[i].originY(), seg[i].before(), seg[i].after());
//         }
    }
    virtual const char * name() const { return "graphiteng"; }
private:
    int m_rtl;
    gr2::FileFaceHandle * m_fileFace;
    gr2::GrFace * m_grFace;
    gr2::GrFont * m_grFont;
};
