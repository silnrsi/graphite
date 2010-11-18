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
#include "GrFontImp.h"

using namespace org::sil::graphite::v2;

GrFont::GrFont(float ppm, const GrFace *face) :
    m_face(face),
    m_scale(ppm / face->upem())
{
    size_t nGlyphs=m_face->numGlyphs();
    m_advances = gralloc<float>(nGlyphs);
    float *advp = m_advances;
    for (size_t i = 0; i < nGlyphs; i++)
    { *advp++ = INVALID_ADVANCE; }
}


/*virtual*/ GrFont::~GrFont()
{
    free(m_advances);
}


/*virtual*/ float GrFont::computeAdvance(unsigned short glyphid) const
{
    return m_face->getAdvance(glyphid, m_scale);
}



GrHintedFont::GrHintedFont(float ppm/*pixels per em*/, const void* appFontHandle/*non-NULL*/, advance_fn advance2, const GrFace *face) :
    GrFont(ppm, face), 
    m_appFontHandle(appFontHandle),
    m_advance(advance2)
{
}


/*virtual*/ float GrHintedFont::computeAdvance(unsigned short glyphid) const
{
    return (*m_advance)(m_appFontHandle, glyphid);
}



