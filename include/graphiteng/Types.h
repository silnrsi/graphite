#pragma once

#include <cstddef>

namespace org { namespace sil { namespace graphite { namespace v2 {

typedef unsigned char uint8;
typedef uint8    byte;
typedef signed char int8;
typedef unsigned short uint16;
typedef short   int16;
typedef unsigned int    uint32;
typedef int     int32;

#ifdef _MSC_VER
#define GRNG_EXPORT __declspec(dllexport)
#else
#ifdef __GNUC__
#define GRNG_EXPORT __attribute__ ((visibility("default")))
#else
#define GRNG_EXPORT
#endif
#endif

#define FIND_BROKEN_VIRTUALS

}}}} // namespace

