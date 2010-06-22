
#include <cstring>
#include <cassert>
#include <stdarg.h>

#include "XmlTraceLog.h"

XmlTraceLog XmlTraceLog::sNullLog(NULL, "", GRLOG_NONE);
XmlTraceLog * XmlTraceLog::sLog = &XmlTraceLog::sNullLog;

void startGraphiteLogging(FILE * logFile, GrLogMask mask)
{
    if (XmlTraceLog::sLog != &XmlTraceLog::sNullLog)
    {
        delete XmlTraceLog::sLog;
    }
    XmlTraceLog::sLog = new XmlTraceLog(logFile, "http://projects.palaso.org/graphiteng", mask);
}

void stopGraphiteLogging()
{
    if (XmlTraceLog::sLog)
    {
        delete XmlTraceLog::sLog;
        XmlTraceLog::sLog = &XmlTraceLog::sNullLog;
    }
}



XmlTraceLog::XmlTraceLog(FILE * file, const char * ns, GrLogMask logMask)
                         : mFile(file), mDepth(0), mMask(logMask)
{
    if (!mFile) return;
    fprintf(mFile, "<?xml version='1.0' encoding='UTF-8'?>\n<%s xmlns='%s'>",
            xmlTraceLogElements[ElementTopLevel].mName, ns);
    mElementStack[mDepth++] = ElementTopLevel;
    mElementEmpty = true;
    mInElement = false;
    mLastNodeText = false;
}

XmlTraceLog::~XmlTraceLog()
{
    if (mFile && mFile != stdout && mFile != stderr)
    {
        assert(mDepth == 1);
        while (mDepth > 0)
        {
            closeElement(mElementStack[mDepth-1]);
        }
        fclose(mFile);
        mFile = NULL;
    }
}

void XmlTraceLog::openElement(XmlTraceLogElement eId)
{
    if (!mFile) return;
    if (mInElement)
    {
        if (xmlTraceLogElements[mElementStack[mDepth-1]].mFlags & mMask)
            fprintf(mFile, ">");
    }
    if (xmlTraceLogElements[eId].mFlags & mMask)
    {
        if (!mLastNodeText)
        {
            fprintf(mFile, "\n");
            for (size_t i = 0; i < mDepth; i++)
            {
                fprintf(mFile, " ");
            }
        }
        fprintf(mFile, "<%s", xmlTraceLogElements[eId].mName);
    }
    mElementStack[mDepth++] = eId;
    mInElement = true;
    mLastNodeText = false;
}

void XmlTraceLog::closeElement(XmlTraceLogElement eId)
{
    if (!mFile) return;
    assert(mDepth > 0);
    assert(eId == mElementStack[mDepth-1]);
    --mDepth;
    if (xmlTraceLogElements[eId].mFlags & mMask)
    {
        if (mInElement)
        {
            fprintf(mFile, "/>");
        }
        else
        {
            if (!mLastNodeText)
            {
                fprintf(mFile, "\n");
                for (size_t i = 0; i < mDepth; i++)
                    fprintf(mFile, " ");
            }
            fprintf(mFile, "</%s>", xmlTraceLogElements[eId].mName);
        }
    }
    mInElement = false;
    mLastNodeText = false;
}

void XmlTraceLog::addAttribute(XmlTraceLogAttribute aId, const char * value)
{
    if (!mFile) return;
    assert(mInElement);
    if (xmlTraceLogElements[mElementStack[mDepth-1]].mFlags & mMask)
    {
        fprintf(mFile, " %s=\"", xmlTraceLogAttributes[aId]);
        escapeIfNeeded(value);
        fprintf(mFile, "\"");
    }
}

void XmlTraceLog::addAttribute(XmlTraceLogAttribute aId, float value)
{
    if (!mFile) return;
    assert(mInElement);
    if (xmlTraceLogElements[mElementStack[mDepth-1]].mFlags & mMask)
    {
        fprintf(mFile, " %s=\"%f\"", xmlTraceLogAttributes[aId], value);
    }
}

void XmlTraceLog::addAttribute(XmlTraceLogAttribute aId, byte value)
{
    if (!mFile) return;
    assert(mInElement);
    if (xmlTraceLogElements[mElementStack[mDepth-1]].mFlags & mMask)
    {
        fprintf(mFile, " %s=\"%u\"", xmlTraceLogAttributes[aId], (uint32)value);
    }
}

void XmlTraceLog::addAttribute(XmlTraceLogAttribute aId, int32 value)
{
    if (!mFile) return;
    assert(mInElement);
    if (xmlTraceLogElements[mElementStack[mDepth-1]].mFlags & mMask)
    {
        fprintf(mFile, " %s=\"%d\"", xmlTraceLogAttributes[aId], value);
    }
}

void XmlTraceLog::addAttribute(XmlTraceLogAttribute aId, uint32 value)
{
    if (!mFile) return;
    assert(mInElement);
    if (xmlTraceLogElements[mElementStack[mDepth-1]].mFlags & mMask)
    {
        fprintf(mFile, " %s=\"%u\"", xmlTraceLogAttributes[aId], value);
    }
}

void XmlTraceLog::addAttributeFixed(XmlTraceLogAttribute aId, uint32 value)
{
    if (!mFile) return;
    assert(mInElement);
    if (xmlTraceLogElements[mElementStack[mDepth-1]].mFlags & mMask)
    {
        uint32 whole = (value >> 16);
        float fraction = static_cast<float>(value & 0xFFFF) / static_cast<float>(0x1FFFE);
        float fixed = whole + fraction;
        fprintf(mFile, " %s=\"%f\"", xmlTraceLogAttributes[aId], fixed);
    }
}

void XmlTraceLog::addAttribute(XmlTraceLogAttribute aId, int16 value)
{
    if (!mFile) return;
    assert(mInElement);
    if (xmlTraceLogElements[mElementStack[mDepth-1]].mFlags & mMask)
    {
        fprintf(mFile, " %s=\"%d\"", xmlTraceLogAttributes[aId], (int)value);
    }
}

void XmlTraceLog::addAttribute(XmlTraceLogAttribute aId, uint16 value)
{
    if (!mFile) return;
    assert(mInElement);
    if (xmlTraceLogElements[mElementStack[mDepth-1]].mFlags & mMask)
    {
        fprintf(mFile, " %s=\"%u\"", xmlTraceLogAttributes[aId], (unsigned int)value);
    }
}

void XmlTraceLog::writeText(const char * utf8)
{
    if (!mFile) return;
    if (mInElement)
    {
        if (xmlTraceLogElements[mElementStack[mDepth-1]].mFlags & mMask)
        {
            fprintf(mFile, ">");
        }
        mInElement = false;
    }
    if (xmlTraceLogElements[mElementStack[mDepth-1]].mFlags & mMask)
    {
        escapeIfNeeded(utf8);
    }
    mLastNodeText = true;
}

void XmlTraceLog::writeUnicode(const uint32 code)
{
    if (!mFile) return;
    if (mInElement)
    {
        if (xmlTraceLogElements[mElementStack[mDepth-1]].mFlags & mMask)
        {
            fprintf(mFile, ">");
        }
        mInElement = false;
    }
    if (xmlTraceLogElements[mElementStack[mDepth-1]].mFlags & mMask)
    {
        fprintf(mFile, "&#x%02x;", code);
    }
}

void XmlTraceLog::escapeIfNeeded(const char * data)
{
    size_t length = strlen(data);
    for (size_t i = 0; i < length; i++)
    {
        switch (data[i])
        {
            case '<':
                fprintf(mFile, "&lt;");
                break;
            case '>':
                fprintf(mFile, "&gt;");
                break;
            case '&':
                fprintf(mFile, "&amp;");
                break;
            case '"':
                fprintf(mFile, "&#34;");
                break;
            default:
                fprintf(mFile, "%c", data[i]);
        }
    }
}

static const int MAX_MSG_LEN = 1024;

void XmlTraceLog::error(const char * msg, ...)
{
    if (!mFile) return;
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
    if (!mFile) return;
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
