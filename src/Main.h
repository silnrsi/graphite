#pragma once

#include <cstdlib>
#include "graphiteng/Types.h"

#define MAKE_TAG(a,b,c,d) ((a << 24UL) + (b << 16UL) + (c << 8UL) + (d))

#if !defined WORDS_BIGENDIAN || defined PC_OS
#define swap16(x) (((x & 0xff) << 8) | ((x & 0xff00) >> 8))
#define swap32(x) (((x & 0xff) << 24) | ((x & 0xff00) << 8) | ((x & 0xff0000) >> 8) | ((x & 0xff000000) >> 24))
#else
#define swap16(x) (x)
#define swap32(x) (x)
#endif

#define read16(x) (x+=sizeof(uint16), swap16(*(uint16 *)(x-sizeof(uint16))))
#define read32(x) (x+=sizeof(uint32), swap32(*(uint32 *)(x-sizeof(uint32))))

#define CLASS_NEW_DELETE \
    void * operator new[](size_t size) {return malloc(size);} \
    void operator delete[] (void * p){ if (p) free(p); } \
    void * operator new(size_t size){ return malloc(size);} \
    void operator delete (void * p) {if (p) free(p);}

namespace org { namespace sil { namespace graphite { namespace v2 {

    // typesafe wrapper around malloc for simple types
    // use free(pointer) to deallocate
    template <typename T> T * gralloc(size_t n)
    {
        return reinterpret_cast<T*>(malloc(sizeof(T) * n));
    }

}}}} // namespace
