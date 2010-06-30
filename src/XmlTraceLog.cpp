
#include <cstring>
#include <cassert>
#include <stdarg.h>

#include "XmlTraceLog.h"

#ifndef DISABLE_TRACING

static NullTraceLog s_NullLog;
XmlTraceLog * XmlTraceLog::sLog = &s_NullLog;



XmlTraceLog::XmlTraceLog(FILE * file, const char * ns, GrLogMask logMask)
                         : m_file(file), m_depth(0), m_mask(logMask)
{
    if (!m_file) return;
    int deep = 0;
#ifdef ENABLE_DEEP_TRACING
    deep = 1;
#endif
    fprintf(m_file, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<%s xmlns=\"%s\" mask=\"%x\" deep=\"%d\">",
            xmlTraceLogElements[ElementTopLevel].mName, ns, logMask, deep);
    m_elementStack[m_depth++] = ElementTopLevel;
    m_elementEmpty = true;
    m_inElement = false;
    m_lastNodeText = false;
}

XmlTraceLog::~XmlTraceLog()
{
    if (m_file && m_file != stdout && m_file != stderr)
    {
        assert(m_depth == 1);
        while (m_depth > 0)
        {
            closeElement(m_elementStack[m_depth-1]);
        }
        fclose(m_file);
        m_file = NULL;
    }
}

void XmlTraceLog::openElement(XmlTraceLogElement eId)
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

void XmlTraceLog::closeElement(XmlTraceLogElement eId)
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

void XmlTraceLog::addArrayElement(XmlTraceLogElement eId, const byte *start, int num)
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
        fprintf(m_file, "<%s>\n", xmlTraceLogElements[eId].mName);
	while (num-- > 0)
	{
	    for (size_t i = 0; i < m_depth + 1; i++) fprintf(m_file, " ");
	    fprintf(m_file, "<val>%d</val>\n", (uint32)*start++);
	}
	for (size_t i = 0; i < m_depth; i++) fprintf(m_file, " ");
        fprintf(m_file, "</%s>", xmlTraceLogElements[eId].mName);
	m_inElement = false;
	m_lastNodeText = false;
    }
}

void XmlTraceLog::addSingleElement(XmlTraceLogElement eId, const int value)
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
        fprintf(m_file, "<%s val=\"%d\"/>", xmlTraceLogElements[eId].mName, value);
	m_inElement = false;
	m_lastNodeText = false;
    }
}
    
void XmlTraceLog::addAttribute(XmlTraceLogAttribute aId, const char * value)
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

void XmlTraceLog::addAttribute(XmlTraceLogAttribute aId, float value)
{
    if (!m_file) return;
    assert(m_inElement);
    if (xmlTraceLogElements[m_elementStack[m_depth-1]].mFlags & m_mask)
    {
        fprintf(m_file, " %s=\"%f\"", xmlTraceLogAttributes[aId], value);
    }
}

void XmlTraceLog::addAttribute(XmlTraceLogAttribute aId, int value)
{
    if (!m_file) return;
    assert(m_inElement);
    if (xmlTraceLogElements[m_elementStack[m_depth-1]].mFlags & m_mask)
    {
        fprintf(m_file, " %s=\"%d\"", xmlTraceLogAttributes[aId], value);
    }
}

void XmlTraceLog::addAttribute(XmlTraceLogAttribute aId, unsigned int value)
{
    if (!m_file) return;
    assert(m_inElement);
    if (xmlTraceLogElements[m_elementStack[m_depth-1]].mFlags & m_mask)
    {
        fprintf(m_file, " %s=\"%u\"", xmlTraceLogAttributes[aId], value);
    }
}


void XmlTraceLog::addAttributeFixed(XmlTraceLogAttribute aId, uint32 value)
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

void XmlTraceLog::writeElementArray(XmlTraceLogElement eId, XmlTraceLogAttribute aId, int16 values [], size_t length)
{
    if (!m_file) return;
    if (xmlTraceLogElements[eId].mFlags & m_mask)
    {
        if(m_inElement && xmlTraceLogElements[m_elementStack[m_depth-1]].mFlags & m_mask)
        {
            fprintf(m_file, ">");
            m_inElement = false;
        }
        // line break after 5 columns
        for (size_t i = 0; i < length; i++)
        {
            if (i % 5 == 0)
            {
                fprintf(m_file, "\n");
                for (size_t j = 0; j < m_depth; j++)
                {
                    fprintf(m_file, " ");
                }
            }
            fprintf(m_file, "<%s index=\"%d\" %s=\"%d\"/>", xmlTraceLogElements[eId].mName, int(i),
                xmlTraceLogAttributes[aId], (int)values[i]);
        }
    }
}

void XmlTraceLog::writeText(const char * utf8)
{
    if (!m_file) return;
    if (m_inElement)
    {
        if (xmlTraceLogElements[m_elementStack[m_depth-1]].mFlags & m_mask)
        {
            fprintf(m_file, ">");
        }
        m_inElement = false;
    }
    if (xmlTraceLogElements[m_elementStack[m_depth-1]].mFlags & m_mask)
    {
        escapeIfNeeded(utf8);
    }
    m_lastNodeText = true;
}

void XmlTraceLog::writeUnicode(const uint32 code)
{
    if (!m_file) return;
    if (m_inElement)
    {
        if (xmlTraceLogElements[m_elementStack[m_depth-1]].mFlags & m_mask)
        {
            fprintf(m_file, ">");
        }
        m_inElement = false;
    }
    if (xmlTraceLogElements[m_elementStack[m_depth-1]].mFlags & m_mask)
    {
        fprintf(m_file, "&#x%02x;", code);
    }
    m_lastNodeText = true;
}

void XmlTraceLog::escapeIfNeeded(const char * data)
{
    size_t length = strlen(data);
    for (size_t i = 0; i < length; i++)
    {
        switch (data[i])
        {
            case '<':
                fprintf(m_file, "&lt;");
                break;
            case '>':
                fprintf(m_file, "&gt;");
                break;
            case '&':
                fprintf(m_file, "&amp;");
                break;
            case '"':
                fprintf(m_file, "&#34;");
                break;
            default:
                fprintf(m_file, "%c", data[i]);
        }
    }
}

static const int MAX_MSG_LEN = 1024;

void XmlTraceLog::error(const char * msg, ...)
{
    if (!m_file) return;
    openElement(ElementError);
    va_list args;
    va_start(args, msg);
    char buffer[MAX_MSG_LEN];
    int len = vsnprintf(buffer, MAX_MSG_LEN, msg, args);
    assert(len + 1 < MAX_MSG_LEN);
    writeText(buffer);
    va_end(args);
    closeElement(ElementError);
}

void XmlTraceLog::warning(const char * msg, ...)
{
    if (!m_file) return;
    openElement(ElementWarning);
    va_list args;
    va_start(args, msg);
    char buffer[MAX_MSG_LEN];
    int len = vsnprintf(buffer, MAX_MSG_LEN, msg, args);
    assert(len + 1 < MAX_MSG_LEN);
    writeText(buffer);
    va_end(args);
    closeElement(ElementWarning);
}

#endif		//!DISABLE_TRACING


void startGraphiteLogging(FILE * logFile, GrLogMask mask)
{
#ifdef DISABLE_TRACING
    logFile;			//pointless uses to avoid warnings re implementation not using parameters
    mask;
#else	//!DISABLE_TRACING
    if (XmlTraceLog::sLog != &s_NullLog)
    {
        delete XmlTraceLog::sLog;
    }
    XmlTraceLog::sLog = new XmlTraceLog(logFile, "http://projects.palaso.org/graphiteng", mask);
#endif		//!DISABLE_TRACING
}

void stopGraphiteLogging()
{
#ifndef DISABLE_TRACING
    if (XmlTraceLog::sLog && XmlTraceLog::sLog != &s_NullLog)
    {
        delete XmlTraceLog::sLog;
        XmlTraceLog::sLog = &s_NullLog;
    }
#endif		//!DISABLE_TRACING
}

