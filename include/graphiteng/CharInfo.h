#pragma once

#include "graphiteng/Types.h"

namespace org { namespace sil { namespace graphite { namespace v2 {

class CharInfo;
  
extern "C"
{
    GRNG_EXPORT unsigned int cinfo_unicode_char(const CharInfo* p/*not NULL*/);
    GRNG_EXPORT int cinfo_break_weight(const CharInfo* p/*not NULL*/);
}


}}}} // namespace
