#pragma once

#include <cstdio>
#include <graphiteng/Types.h>


typedef enum {
    GRLOG_NONE = 0x0,
    GRLOG_FACE = 0x01,
    GRLOG_SEGMENT = 0x02,
    GRLOG_PASS = 0x04,
    
    GRLOG_OPCODE = 0x80,
    GRLOG_ALL = 0xFF
} GrLogMask;

extern GRNG_EXPORT void startGraphiteLogging(FILE * logFile, GrLogMask mask);		//may not do anthing if disabled in the implementation of the engine.
extern GRNG_EXPORT void stopGraphiteLogging();
