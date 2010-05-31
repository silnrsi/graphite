#include "Slot.h"

void Slot::update(int numSlots, int numCharInfo, Position &relpos)
{
    if (m_prev) m_prev += numSlots;
    if (m_next) m_next += numSlots;
    m_before += numCharInfo;
    m_after += numCharInfo;
    m_position = m_position + relpos;
}

