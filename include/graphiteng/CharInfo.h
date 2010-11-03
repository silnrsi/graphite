#pragma once

#include "graphiteng/Types.h"

namespace org { namespace sil { namespace graphite { namespace v2 {

class CharInfo;

enum {
    /* after break weights */
    eBreakWhitespace = 10,
    eBreakWord = 15,
    eBreakIntra = 20,
    eBreakLetter = 30,
    eBreakClip = 40,
    /* before break weights */
    eBreakBeforeWhitespace = -10,
    eBreakBeforeWord = -15,
    eBreakBeforeIntra = -20,
    eBreakBeforeLetter = -30,
    eBreakBeforeClip = -40
};

extern "C"
{
    GRNG_EXPORT unsigned int unicode_char(const CharInfo* p/*not NULL*/);
    GRNG_EXPORT int break_weight(const CharInfo* p/*not NULL*/);
}


}}}} // namespace
