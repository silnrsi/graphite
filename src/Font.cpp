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
#include "Font.h"


Font::Font(float ppm, const Face *face/*needed for scaling*/) :
    m_scale(ppm / face->upem())
{
    size_t nGlyphs=face->numGlyphs();
    m_advances = gralloc<float>(nGlyphs);
    if (m_advances)
    {
        float *advp = m_advances;
        for (size_t i = 0; i < nGlyphs; i++)
        { *advp++ = INVALID_ADVANCE; }
    }
}


/*virtual*/ Font::~Font()
{
    if (m_advances)
        free(m_advances);
}


SimpleFont::SimpleFont(float ppm/*pixels per em*/, const Face *face) :
  Font(ppm, face),
  m_face(face)
{
}
  
  
/*virtual*/ float SimpleFont::computeAdvance(unsigned short glyphid) const
{
    return m_face->getAdvance(glyphid, m_scale);
}



HintedFont::HintedFont(float ppm/*pixels per em*/, const void* appFontHandle/*non-NULL*/, gr_advance_fn advance2, const Face *face/*needed for scaling*/) :
    Font(ppm, face), 
    m_appFontHandle(appFontHandle),
    m_advance(advance2)
{
}


/*virtual*/ float HintedFont::computeAdvance(unsigned short glyphid) const
{
    return (*m_advance)(m_appFontHandle, glyphid);
}



