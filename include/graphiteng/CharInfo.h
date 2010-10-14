#pragma once

#include "graphiteng/Types.h"

namespace org { namespace sil { namespace graphite { namespace v2 {

class CharInfo;
  
extern "C"
{
    GRNG_EXPORT unsigned int unicode_char(const CharInfo* p/*not NULL*/);
    GRNG_EXPORT int break_weight(const CharInfo* p/*not NULL*/);
}


}}}} // namespace
