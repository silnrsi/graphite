// XmlTraceLog - produces a log in XML format
#ifndef XmlTraceLog_h
#define XmlTraceLog_h
#include <cstdio>
#include <graphiteng/Types.h>
#include "XmlTraceLogTags.h"
#include <graphiteng/XmlLog.h>

class XmlTraceLog
{
    friend void startGraphiteLogging(FILE * logFile, GrLogMask mask);
    friend void stopGraphiteLogging();
public:
    ~XmlTraceLog();
    bool active() { return (mFile != NULL); };
    void openElement(XmlTraceLogElement eId);
    void closeElement(XmlTraceLogElement eId);
    void addAttribute(XmlTraceLogAttribute aId, const char * value);
    void addAttribute(XmlTraceLogAttribute aId, byte value);
    void addAttribute(XmlTraceLogAttribute aId, float value);
    void addAttribute(XmlTraceLogAttribute aId, int32 value);
    void addAttribute(XmlTraceLogAttribute aId, uint32 value);
    void addAttributeFixed(XmlTraceLogAttribute aId, uint32 value);
    void addAttribute(XmlTraceLogAttribute aId, int16 value);
    void addAttribute(XmlTraceLogAttribute aId, uint16 value);
    void writeText(const char * utf8);
    void writeUnicode(const uint32 code);
    void error(const char * msg, ...);
    void warning(const char * msg, ...);
    static XmlTraceLog & get()
    {
        if (sLog == NULL)
        {
            sLog = new XmlTraceLog(NULL, "", GRLOG_NONE);
        }
        return *sLog;
    }
protected:
    static XmlTraceLog * sLog;    
private:
    XmlTraceLog(FILE * file, const char * ns, GrLogMask logMask);
    void escapeIfNeeded(const char * text);
    enum {
        MAX_ELEMENT_DEPTH = 256
    };
    FILE * mFile;
    bool mInElement;
    bool mElementEmpty;
    bool mLastNodeText;
    uint32 mDepth;
    GrLogMask mMask;
    XmlTraceLogElement mElementStack[MAX_ELEMENT_DEPTH];
};

#endif
