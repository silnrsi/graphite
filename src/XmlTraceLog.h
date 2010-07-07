#pragma once
// XmlTraceLog - produces a log in XML format

#include <cstdio>
#include <graphiteng/Types.h>
#include "XmlTraceLogTags.h"
#include <graphiteng/XmlLog.h>
#include <cassert>

#ifndef DISABLE_TRACING

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
private:
    static XmlTraceLog sm_NullLog;
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


inline void XmlTraceLog::openElement(XmlTraceLogElement eId)
{
    if (!m_file) return;
    if (m_inElement)
    {
        if (xmlTraceLogElements[m_elementStack[m_depth-1]].mFlags & m_mask)
            fprintf(m_file, ">");
    }
    if (xmlTraceLogElements[eId].mFlags & m_mask)
    {
        if (!m_lastNodeText)
        {
            fprintf(m_file, "\n");
            for (size_t i = 0; i < m_depth; i++)
            {
                fprintf(m_file, " ");
            }
        }
        fprintf(m_file, "<%s", xmlTraceLogElements[eId].mName);
    }
    m_elementStack[m_depth++] = eId;
    m_inElement = true;
    m_lastNodeText = false;
}


inline void XmlTraceLog::closeElement(XmlTraceLogElement eId)
{
    if (!m_file) return;
    assert(m_depth > 0);
    assert(eId == m_elementStack[m_depth-1]);
    --m_depth;
    if (xmlTraceLogElements[eId].mFlags & m_mask)
    {
        if (m_inElement)
        {
            fprintf(m_file, "/>");
        }
        else
        {
            if (!m_lastNodeText)
            {
                fprintf(m_file, "\n");
                for (size_t i = 0; i < m_depth; i++)
                    fprintf(m_file, " ");
            }
            fprintf(m_file, "</%s>", xmlTraceLogElements[eId].mName);
        }
    }
    m_inElement = false;
    m_lastNodeText = false;
#ifdef ENABLE_DEEP_TRACING
    fflush(m_file);
#endif
}


inline void XmlTraceLog::addAttribute(XmlTraceLogAttribute aId, const char * value)
{
    if (!m_file) return;
    assert(m_inElement);
    if (xmlTraceLogElements[m_elementStack[m_depth-1]].mFlags & m_mask)
    {
        fprintf(m_file, " %s=\"", xmlTraceLogAttributes[aId]);
        escapeIfNeeded(value);
        fprintf(m_file, "\"");
    }
}


inline void XmlTraceLog::addAttribute(XmlTraceLogAttribute aId, float value)
{
    if (!m_file) return;
    assert(m_inElement);
    if (xmlTraceLogElements[m_elementStack[m_depth-1]].mFlags & m_mask)
    {
        fprintf(m_file, " %s=\"%f\"", xmlTraceLogAttributes[aId], value);
    }
}


inline void XmlTraceLog::addAttribute(XmlTraceLogAttribute aId, int value)
{
    if (!m_file) return;
    assert(m_inElement);
    if (xmlTraceLogElements[m_elementStack[m_depth-1]].mFlags & m_mask)
    {
        fprintf(m_file, " %s=\"%d\"", xmlTraceLogAttributes[aId], value);
    }
}


inline void XmlTraceLog::addAttribute(XmlTraceLogAttribute aId, unsigned int value)
{
    if (!m_file) return;
    assert(m_inElement);
    if (xmlTraceLogElements[m_elementStack[m_depth-1]].mFlags & m_mask)
    {
        fprintf(m_file, " %s=\"%u\"", xmlTraceLogAttributes[aId], value);
    }
}


inline void XmlTraceLog::addAttributeFixed(XmlTraceLogAttribute aId, uint32 value)
{
    if (!m_file) return;
    assert(m_inElement);
    if (xmlTraceLogElements[m_elementStack[m_depth-1]].mFlags & m_mask)
    {
        uint32 whole = (value >> 16);
        float fraction = static_cast<float>(value & 0xFFFF) / static_cast<float>(0x1FFFE);
        float fixed = whole + fraction;
        fprintf(m_file, " %s=\"%f\"", xmlTraceLogAttributes[aId], fixed);
    }
}




#endif
