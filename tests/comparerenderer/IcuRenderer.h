/*  GRAPHITE2 LICENSING

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
    If not, write to the Free Software Foundation, 51 Franklin Street,
    Suite 500, Boston, MA 02110-1335, USA or visit their web page on the
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
        m_font(fontFile, static_cast<float>(fontSize), m_status),
        m_scriptCode(USCRIPT_UNKNOWN), m_eIcu(NULL)
    {
    }

    virtual ~IcuRenderer()
    {
        delete m_eIcu;
    }

    virtual void renderText(const char *utf8, size_t length, RenderedLine *result, FILE *log)
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
        le_int32 typographyFlags = 3; // essential for ligatures and kerning
        if (m_eIcu == NULL || scriptCode != m_scriptCode)
        {
            m_eIcu  = LayoutEngine::layoutEngineFactory(&m_font, scriptCode, langCode, typographyFlags, status);
            m_scriptCode = scriptCode;
        }
        if (!m_eIcu || LE_FAILURE(status))
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
        int32_t glyphCount = m_eIcu->layoutChars(uText.getBuffer(), 0, count, count, m_rtl, 0., 0., status);
        LEGlyphID *glyphs = new LEGlyphID[glyphCount];
        le_int32 *indices = new le_int32[glyphCount];
        float *positions = new float[glyphCount * 2 + 2];
        m_eIcu->getGlyphs(glyphs, status);
        m_eIcu->getCharIndices(indices, status);
        m_eIcu->getGlyphPositions(positions, status);

        while ((glyphCount > 0) && (glyphs[glyphCount-1] == 65535))
        {
            --glyphCount;
        }
        RenderedLine *renderedLine = new(result) RenderedLine(glyphCount, positions[2*glyphCount]);
        int j = 0;
        for (i = 0; i < glyphCount; ++i)
        {
            if (glyphs[i] == 65535)
                continue;
            (*renderedLine)[j++].set(glyphs[i], positions[2*i], -positions[2*i + 1],
                                    indices[i], indices[i]);
        }
        renderedLine->resize(j);
        delete[] glyphs;
        delete[] indices;
        delete[] positions;
        m_eIcu->reset();
    }

    virtual const char *name() const { return "ICU"; }

private :
    int m_rtl;
    LEErrorCode m_status;
    PortableFontInstance m_font;
    UScriptCode m_scriptCode;
    LayoutEngine * m_eIcu;
};
