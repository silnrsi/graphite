#pragma once
// XmlTraceLog - produces a log in XML format

#include <cstdio>
#include <graphiteng/Types.h>
#include "XmlTraceLogTags.h"
#include <graphiteng/XmlLog.h>

#ifndef DISABLE_TRACING

class NullTraceLog;

class XmlTraceLog
{
    friend bool startGraphiteLogging(FILE * logFile, GrLogMask mask);
    friend void stopGraphiteLogging();
public:
    ~XmlTraceLog();
    bool active() { return (m_file != NULL); };
    void openElement(XmlTraceLogElement eId);
    void closeElement(XmlTraceLogElement eId);
    void addArrayElement(XmlTraceLogElement eId, const byte *start, int num);
    void addSingleElement(XmlTraceLogElement eId, const int value);
    void addAttribute(XmlTraceLogAttribute aId, const char * value);
    void addAttribute(XmlTraceLogAttribute aId, float value);
    void addAttribute(XmlTraceLogAttribute aId, int value);
    void addAttribute(XmlTraceLogAttribute aId, unsigned int value);
#if SIZEOF_SIZE_T == 8
    void addAttribute(XmlTraceLogAttribute aId, size_t value) { addAttribute(aId, (uint32)value); }
#endif
    void addAttributeFixed(XmlTraceLogAttribute aId, uint32 value);
    void writeText(const char * utf8);
    void writeUnicode(const uint32 code);
    void writeElementArray(XmlTraceLogElement eId, XmlTraceLogAttribute aId, int16 * values, size_t length);
    void error(const char * msg, ...);
    void warning(const char * msg, ...);
    static XmlTraceLog & get()
    {
        return *sLog;
    }
protected:
    static XmlTraceLog * sLog;
    XmlTraceLog(FILE * file, const char * ns, GrLogMask logMask);
private:
    void escapeIfNeeded(const char * text);
    enum {
        MAX_ELEMENT_DEPTH = 256
    };
    FILE * m_file;
    bool m_inElement;
    bool m_elementEmpty;
    bool m_lastNodeText;
    uint32 m_depth;
    GrLogMask m_mask;
    XmlTraceLogElement m_elementStack[MAX_ELEMENT_DEPTH];
};

class NullTraceLog : public XmlTraceLog
{
public:
    NullTraceLog() : XmlTraceLog(NULL, NULL, GRLOG_NONE){};
};

#endif
