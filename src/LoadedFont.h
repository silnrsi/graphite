#pragma once

#include "graphiteng/IFont.h"
#include "LoadedFace.h"

const float INVALID_ADVANCE = -1e38f;		//because this is in the header it can be optimized out.

class LoadedFont
{
public:
     LoadedFont(float ppm, const LoadedFace *face);
private:
    void initialize();
public:
    virtual ~LoadedFont();
    float advance(unsigned short glyphid) const {
        if (m_advances[glyphid] == INVALID_ADVANCE)
            m_advances[glyphid] = computeAdvance(glyphid);
        return m_advances[glyphid];
    }
//    Position scale(const Position& p) const { return Position(m_scale * p.x, m_scale * p.y); }
//    float scale(float p) const { return m_scale * p; }
    float scale() const { return m_scale; }
    
private:
    virtual float computeAdvance(unsigned short glyphid) const;
    
private:
    float m_scale;      // scales from design units to ppm
    const LoadedFace *m_face;   // LoadedFace to get the rest of the info from
protected:
    float *m_advances;  // One advance per glyph in pixels. Nan if not defined
    
private:			//defensive on m_advances
    LoadedFont(const LoadedFont&);
    LoadedFont& operator=(const LoadedFont&);
};

class LoadedFontWithHints : public LoadedFont
{
public:
   LoadedFontWithHints(const IFont *font/*not NULL*/, const LoadedFace *face);
   
private:
    virtual float computeAdvance(unsigned short glyphid) const;
    
private:
    const IFont *m_font;      // Application interface.
};