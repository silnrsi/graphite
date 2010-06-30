#pragma once

#include "graphiteng/IFont.h"
#include "LoadedFace.h"

const float INVALID_ADVANCE = -1e38f;		//because this is in the header it can be optimized out.

class LoadedFont
{
public:
    LoadedFont(const IFont *font/*not NULL*/, const LoadedFace *face);
    LoadedFont(float ppm, const LoadedFace *face);
private:
    void initialize();
public:
    ~LoadedFont();
    float advance(unsigned short glyphid) const {
        if (m_advances[glyphid] == INVALID_ADVANCE)
            m_advances[glyphid] = m_font ? m_font->advance(glyphid) : m_face->getAdvance(glyphid, m_scale);
        return m_advances[glyphid];
    }
    Position scale(const Position& p) const { return Position(m_scale * p.x, m_scale * p.y); }
    float scale(float p) const { return m_scale * p; }
    float scale() const { return m_scale; }

private:
    const IFont *m_font;      // Application interface. May be NULL
    float m_scale;      // scales from design units to ppm
    float *m_advances;  // One advance per glyph in pixels. Nan if not defined
    const LoadedFace *m_face;   // LoadedFace to get the rest of the info from
    
private:			//defensive on m_advances
    LoadedFont(const LoadedFont&);
    LoadedFont& operator=(const LoadedFont&);
};

