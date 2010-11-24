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
#pragma once

#include "graphiteng/Types.h"

namespace org { namespace sil { namespace graphite { namespace v2 {

class GrFace;
class GrFont;

extern "C"
{
    GRNG_EXPORT GrFont* make_font(float ppm/*pixels per em*/, const GrFace *face/*needed for scaling, and the advance hints - must stay alive*/);
                      //When finished with the GrFont, call destroy_font    
    typedef float (*advance_fn)(const void* appFontHandle, uint16 glyphid);     //amount to advance. positive is in the writing direction
    GRNG_EXPORT GrFont* make_font_with_advance_fn(float ppm/*pixels per em*/, const void* appFontHandle/*non-NULL*/, advance_fn advance, const GrFace *face/*needed for scaling*/);
                      //the appFontHandle must stay alive all the time when the GrFont is alive. When finished with the GrFont, call destroy_font    
    GRNG_EXPORT void destroy_font(GrFont *font);
}

}}}} // namespace
