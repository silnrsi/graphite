#pragma once

#include "Renderer.h"
#include <graphiteng/Types.h>
#include <graphiteng/IFace.h>
#include <graphiteng/IFont.h>
#include <graphiteng/SegmentHandle.h>

namespace gr2 = org::sil::graphite::v2;

class GrNgRenderer : public Renderer
{
public:
    GrNgRenderer(const char * fontFile, int fontSize, int textDir)
        : m_ttfFace(gr2::TtfFileFace::loadTTFFile(fontFile)),
        m_grFace(m_ttfFace->makeGrFace()),
        m_grFont(gr2::IFont::makeGrFont(fontSize, m_grFace))
    {

    }
    virtual ~GrNgRenderer()
    {
        gr2::IFont::destroyGrFont(m_grFont);
        gr2::IFace::destroyGrFace(m_grFace);
        m_grFont = NULL;
        m_grFace = NULL;
        delete m_ttfFace;
        m_ttfFace = NULL;
    }
    virtual void renderText(const char * utf8, size_t length, RenderedLine * result)
    {
        gr2::SegmentHandle seg(m_grFont, m_grFace, 0u, gr2::SegmentHandle::kutf8, utf8, length, m_rtl);
        RenderedLine * renderedLine = new(result) RenderedLine(seg.length(), seg.advanceX());
        for (size_t i = 0; i < seg.length(); i++)
        {
            (*renderedLine)[i].set(seg[i].gid(), seg[i].originX(), seg[i].originY(), seg[i].before(), seg[i].after());
        }
    }
    virtual const char * name() const { return "graphiteng"; }
private:
    int m_rtl;
    gr2::TtfFileFace * m_ttfFace;
    gr2::GrFace * m_grFace;
    gr2::GrFont * m_grFont;
};
