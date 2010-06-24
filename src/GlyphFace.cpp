#include "GlyphFace.h"
#include "XmlTraceLog.h"


void GlyphFace::readAttrs(const void *pGlat, int start, int end, size_t num) 
{
    m_attrs = new unsigned short[num];
    while (start < end)
    {
        int attr = ((char *)pGlat)[start];
        int count = ((char *)pGlat)[start + 1];
        for (int i = 0; i < count; i++)
        {
            m_attrs[attr + i] = swap16(((uint16 *)((char *)pGlat + start))[1 + i]);
#ifndef DISABLE_TRACING
            XmlTraceLog::get().openElement(ElementAttr);
            XmlTraceLog::get().addAttribute(AttrAttrId, attr + i);
            XmlTraceLog::get().addAttribute(AttrAttrVal, m_attrs[attr+i]);
            XmlTraceLog::get().closeElement(ElementAttr);
#endif
        }
        start += 2 * (count + 1);
    }
}

