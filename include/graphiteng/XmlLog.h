#pragma once

#include <cstdio>
#include <graphiteng/Types.h>

namespace org { namespace sil { namespace graphite { namespace v2 {

typedef enum {
    GRLOG_NONE = 0x0,
    GRLOG_FACE = 0x01,
    GRLOG_SEGMENT = 0x02,
    GRLOG_PASS = 0x04,
    
    GRLOG_OPCODE = 0x80,
    GRLOG_ALL = 0xFF
} GrLogMask;

// If startGraphiteLogging returns true, logging is enabled and the FILE handle
// will be closed by graphite when stopGraphiteLogging is called.
extern "C"
{
extern GRNG_EXPORT bool graphite_start_logging(FILE * logFile, GrLogMask mask);		//may not do anthing if disabled in the implementation of the engine.
extern GRNG_EXPORT void graphite_stop_logging();
}

}}}} // namespace
