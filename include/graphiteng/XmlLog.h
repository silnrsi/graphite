#ifndef XmlLog_h
#define XmlLog_h

#include <cstdio>
#include <graphiteng/Types.h>

typedef enum {
    GRLOG_NONE = 0x0,
    GRLOG_FACE = 0x01,
    GRLOG_SEGMENT = 0x02,
    GRLOG_ALL = 0xFF
} GrLogMask;

extern GRNG_EXPORT void startGraphiteLogging(FILE * logFile, GrLogMask mask);
extern GRNG_EXPORT void stopGraphiteLogging();

#endif
