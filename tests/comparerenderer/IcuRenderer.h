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

#include <new>
#include "Renderer.h"
#include "icule/PortableFontInstance.h"

#include "unicode/utypes.h"
#include "unicode/unistr.h"
#include "unicode/uscript.h"

#include "layout/LETypes.h"
#include "layout/LayoutEngine.h"

class IcuRenderer : public Renderer
{
public:
    IcuRenderer(const char * fontFile, int fontSize, int textDir)
        : m_rtl(textDir), m_status(LE_NO_ERROR),
        m_font(fontFile, static_cast<float>(fontSize), m_status)
    {
    }

    virtual ~IcuRenderer()
    {
    }

    virtual void renderText(const char *utf8, size_t length, RenderedLine *result)
    {
        UScriptCode scriptCode = USCRIPT_UNKNOWN;
        LEErrorCode status = LE_NO_ERROR;
        if (length == 0 || LE_FAILURE(m_status))
        {
            new(result) RenderedLine(0, .0f);
            return;
        }
        le_int32 langCode = -1;
        int i = 0, count = 0;
        while (i < static_cast<int>(length))
        {
            UErrorCode ustatus = U_ZERO_ERROR;
            UChar32 c;
            U8_NEXT(utf8, i, static_cast<int>(length), c);
            if (i == 3 && c == 0xFEFF)
            {
                utf8 += 3;
                i = 0;
                length -= 3;
                continue;
            }
            count++;
            if (scriptCode == USCRIPT_UNKNOWN || scriptCode == USCRIPT_INHERITED)
                scriptCode = uscript_getScript(c, &ustatus);
        }
        
        LayoutEngine *eIcu  = LayoutEngine::layoutEngineFactory(&m_font, scriptCode, langCode, status);
        if (!eIcu || LE_FAILURE(status))
        {
            new(result) RenderedLine(0, .0f);
            return;
        }
        UnicodeString uText;    //  = UnicodeString::fromUTF8(utf8);
        for (i = 0; i < static_cast<int>(length); )
        {
            UChar32 c;
            U8_NEXT(utf8, i, static_cast<int>(length), c);
            uText.append(c);
        }
        int32_t glyphCount = eIcu->layoutChars(uText.getBuffer(), 0, count, count, m_rtl, 0., 0., status);
        LEGlyphID *glyphs = new LEGlyphID[glyphCount];
        le_int32 *indices = new le_int32[glyphCount];
        float *positions = new float[glyphCount * 2 + 2];
        eIcu->getGlyphs(glyphs, status);
        eIcu->getCharIndices(indices, status);
        eIcu->getGlyphPositions(positions, status);

        RenderedLine *renderedLine = new(result) RenderedLine(glyphCount, positions[2*glyphCount]);
        for (i = 0; i < glyphCount; ++i)
        {
            (*renderedLine)[i].set(glyphs[i], positions[2*i], positions[2*i + 1],
                                    indices[i], indices[i]);
        }
        delete[] glyphs;
        delete[] indices;
        delete[] positions;
        delete eIcu;
    }

    virtual const char *name() const { return "ICU"; }

private :
    int m_rtl;
    LEErrorCode m_status;
    PortableFontInstance m_font;   
};
