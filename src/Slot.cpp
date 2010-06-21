#include "Slot.h"
#include "Segment.h"

Slot::Slot() :
    m_glyphid(0), m_before(0), m_after(0), m_parent(-1), m_child(-1), m_sibling(-1),
    m_position(0, 0), m_advance(-1, -1), m_shift(0, 0)
{
}

void Slot::update(int numSlots, int numCharInfo, Position &relpos)
{
    m_before += numCharInfo;
    m_after += numCharInfo;
    m_position = m_position + relpos;
};

void Slot::finalise(Segment *seg, FontImpl *font, Position &base, float *cMin, float *cMax)
{
    Position shift = font->scale(m_shift);
    m_position = base + shift;
    if (m_parent == -1)
        *cMax = m_position.x + advance(font) - shift.x;
    else
    {
        float tAdv;
        m_position += font->scale(m_attach);
        tAdv = m_position.x + (m_advance.x > 0 ? font->scale(m_advance.x) : advance(font)) - shift.x;
        if (tAdv > *cMax) *cMax = tAdv;
        if (m_position.x - base.x - shift.x < *cMin) *cMin = m_position.x - base.x - shift.x;
    }

    if (m_child != -1)
        (*seg)[m_child].finalise(seg, font, m_position, cMin, cMax);

    if (m_sibling != -1)
        (*seg)[m_sibling].finalise(seg, font, base, cMin, cMax);
}

    
