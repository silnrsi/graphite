#pragma once

#include "graphiteng/font.h"
#include "Main.h"
#include "GrFace.h"

namespace org { namespace sil { namespace graphite { namespace v2 {

const float INVALID_ADVANCE = -1e38f;		//because this is in the header it can be optimized out.

class GrFont
{
public:
     GrFont(float ppm, const GrFace *face);
private:
    void initialize();
public:
    virtual ~GrFont();
    float advance(unsigned short glyphid) const {
        if (m_advances[glyphid] == INVALID_ADVANCE)
            m_advances[glyphid] = computeAdvance(glyphid);
        return m_advances[glyphid];
    }
//    Position scale(const Position& p) const { return Position(m_scale * p.x, m_scale * p.y); }
//    float scale(float p) const { return m_scale * p; }
    float scale() const { return m_scale; }

    CLASS_NEW_DELETE
private:
    virtual float computeAdvance(unsigned short glyphid) const;
    
private:
    float m_scale;      // scales from design units to ppm
    const GrFace *m_face;   // GrFace to get the rest of the info from
protected:
    float *m_advances;  // One advance per glyph in pixels. Nan if not defined
    
private:			//defensive on m_advances
    GrFont(const GrFont&);
    GrFont& operator=(const GrFont&);
};

class GrHintedFont : public GrFont
{
public:
   GrHintedFont(const IFont *font/*not NULL*/, const GrFace *face);

private:
    virtual float computeAdvance(unsigned short glyphid) const;

private:
    const IFont *m_font;      // Application interface.
};

}}}} // namespace
