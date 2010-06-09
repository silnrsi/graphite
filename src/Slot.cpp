#include "Slot.h"

Slot::Slot() :
    m_id(0), m_next(0), m_prev(0), m_glyphid(0), m_before(0), m_after(0),
    m_position(0, 0), m_kern(0, 0), m_shift(0, 0)
{
}

void Slot::update(int numSlots, int numCharInfo, Position &relpos)
{
    if (m_prev) m_prev += numSlots;
    if (m_next) m_next += numSlots;
    m_before += numCharInfo;
    m_after += numCharInfo;
    m_position = m_position + relpos;
};

