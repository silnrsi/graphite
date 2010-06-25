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

uint16 GlyphFace::getMetric(uint8 metric)
{
    switch ((enum metrics)metric)
    {
        case kgmetLsb : return m_bbox.bl.x;
        case kgmetRsb : return (m_advance.x - m_bbox.tr.x);
        case kgmetBbTop : return m_bbox.tr.y;
        case kgmetBbBottom : return m_bbox.bl.y;
        case kgmetBbLeft : return m_bbox.bl.x;
        case kgmetBbRight : return m_bbox.tr.x;
        case kgmetBbHeight: return (m_bbox.tr.y - m_bbox.bl.y);
        case kgmetBbWidth : return (m_bbox.tr.x - m_bbox.bl.x);
        case kgmetAdvWidth : return m_advance.x;
        case kgmetAdvHeight : return m_advance.y;
        default : return 0;
    }
}
