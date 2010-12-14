/*  GRAPHITENG LICENSING

    Copyright 2010, SIL International
    All rights reserved.

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published
    by the Free Software Foundation; either version 2.1 of License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should also have received a copy of the GNU Lesser General Public
    License along with this library in the file named "LICENSE".
    If not, write to the Free Software Foundation, Inc., 59 Temple Place, 
    Suite 330, Boston, MA 02111-1307, USA or visit their web page on the 
    internet at http://www.fsf.org/licenses/lgpl.html.
*/
#include "GlyphFace.h"
#include "XmlTraceLog.h"
#include "GlyphFaceCache.h"
#include "TtfUtil.h"

using namespace org::sil::graphite::v2;

GlyphFace::GlyphFace(const GlyphFaceCacheHeader& hdr, unsigned short glyphid)
:   m_bbox(DoNotInitialize()), m_advance(DoNotInitialize())
{
        if (glyphid < hdr.m_nGlyphsWithGraphics)
        {
            int nLsb, xMin, yMin, xMax, yMax;
            unsigned int nAdvWid;
            size_t locidx = TtfUtil::LocaLookup(glyphid, hdr.m_pLoca, hdr.m_lLoca, hdr.m_pHead);
            void *pGlyph = TtfUtil::GlyfLookup(hdr.m_pGlyf, locidx);
            if (TtfUtil::HorMetrics(glyphid, hdr.m_pHmtx, hdr.m_lHmtx, hdr.m_pHHea, nLsb, nAdvWid))
                m_advance = Position(static_cast<float>(nAdvWid), 0);
            else
                m_advance = Position();
            if (TtfUtil::GlyfBox(pGlyph, xMin, yMin, xMax, yMax))
                m_bbox = Rect(Position(static_cast<float>(xMin), static_cast<float>(yMin)),
//                    Position(static_cast<float>(xMax - xMin), static_cast<float>(yMax - yMin)));
                    Position(static_cast<float>(xMax), static_cast<float>(yMax)));
            else
                m_bbox = Rect();
        }
        else
        {
            m_advance = Position();
            m_bbox = Rect();
        }
#ifndef DISABLE_TRACING
        if (XmlTraceLog::get().active())
        {
            XmlTraceLog::get().openElement(ElementGlyphFace);
            XmlTraceLog::get().addAttribute(AttrGlyphId, glyphid);
            XmlTraceLog::get().addAttribute(AttrAdvanceX, m_advance.x);
            XmlTraceLog::get().addAttribute(AttrAdvanceY, m_advance.y);
        }
#endif
        if (glyphid < hdr.m_nGlyphsWithAttributes)
        {
            int glocs, gloce;
            if (hdr.m_locFlagsUse32Bit)
            {
                glocs = swap32(((uint32 *)hdr.m_pGloc)[2+glyphid]);
                gloce = swap32(((uint32 *)hdr.m_pGloc)[3+glyphid]);
            }
            else
            {
                glocs = swap16(((uint16 *)hdr.m_pGloc)[4+glyphid]);
                gloce = swap16(((uint16 *)hdr.m_pGloc)[5+glyphid]);
            }
            readAttrs(hdr.m_pGlat, glocs, gloce, hdr.m_numAttrs);
        }
        else
            m_attrs = NULL;
#ifndef DISABLE_TRACING
        XmlTraceLog::get().closeElement(ElementGlyphFace);
#endif
}
 




void GlyphFace::readAttrs(const void *pGlat, int start, int end, size_t num) 
{
    m_attrs = grzeroalloc<uint16>(num);
    while (start < end)
    {
        unsigned int attr = ((uint8 *)pGlat)[start];
        unsigned int count = ((uint8 *)pGlat)[start + 1];
        if (attr + count > num)
        {
#ifndef DISABLE_TRACING
            XmlTraceLog::get().warning("Invalid glat entry: attr id %d count %d", attr, count);
#endif
            return;
        }
        for (unsigned int i = 0; i < count; i++)
        {
            m_attrs[attr + i] = swap16(((uint16 *)((char *)pGlat + start))[1 + i]);
#ifndef DISABLE_TRACING
            if (XmlTraceLog::get().active())
            {
                XmlTraceLog::get().openElement(ElementAttr);
                XmlTraceLog::get().addAttribute(AttrAttrId, attr + i);
                XmlTraceLog::get().addAttribute(AttrAttrVal, m_attrs[attr+i]);
                XmlTraceLog::get().closeElement(ElementAttr);
            }
#endif
        }
        start += 2 * (count + 1);
    }
}

uint16 GlyphFace::getMetric(uint8 metric) const
{
    switch ((enum metrics)metric)
    {
        case kgmetLsb : return static_cast<uint16>(m_bbox.bl.x);
        case kgmetRsb : return static_cast<uint16>(m_advance.x - m_bbox.tr.x);
        case kgmetBbTop : return static_cast<uint16>(m_bbox.tr.y);
        case kgmetBbBottom : return static_cast<uint16>(m_bbox.bl.y);
        case kgmetBbLeft : return static_cast<uint16>(m_bbox.bl.x);
        case kgmetBbRight : return static_cast<uint16>(m_bbox.tr.x);
        case kgmetBbHeight: return static_cast<uint16>(m_bbox.tr.y - m_bbox.bl.y);
        case kgmetBbWidth : return static_cast<uint16>(m_bbox.tr.x - m_bbox.bl.x);
        case kgmetAdvWidth : return static_cast<uint16>(m_advance.x);
        case kgmetAdvHeight : return static_cast<uint16>(m_advance.y);
        default : return 0;
    }
}
