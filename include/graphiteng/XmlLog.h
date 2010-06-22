#ifndef XmlLog_h
#define XmlLog_h

#include <cstdio>

typedef enum {
    GRLOG_NONE = 0x0,
    GRLOG_FACE = 0x01,
    GRLOG_SEGMENT = 0x02,
    GRLOG_ALL = 0xFF
} GrLogMask;

extern void startGraphiteLogging(FILE * logFile, GrLogMask mask);
extern void stopGraphiteLogging();

#endif
