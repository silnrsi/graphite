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
#include "Usp10.h"
#include "Renderer.h"
#include "processUTF.h"

class UniscribeRenderer : public Renderer
{
public:
    UniscribeRenderer(const char * fontFile, const char * fontName, int fontSize, int textDir, int cache)
        : m_rtl(textDir),
        m_fontFile(fontFile),
        m_hdc(GetDC(NULL)),
        m_logFont(createLogFontFromName(fontName)),
        m_hFont(NULL), m_hFontOld(NULL), m_script(NULL)
    {
        if (AddFontResourceExA(fontFile, FR_PRIVATE, 0))
        {
            m_hFont = CreateFontIndirectA(&m_logFont);
            m_hFontOld = (HFONT) SelectObject(m_hdc, m_hFont);
        }
    }
    virtual ~UniscribeRenderer()
    {
        SelectObject(m_hdc, m_hFontOld);
    	DeleteObject(m_hFont);
        RemoveFontResourceExA(m_fontFile, FR_PRIVATE, 0);
        DeleteDC(m_hdc);
    	m_hdc = NULL;
    }
    LOGFONTA createLogFontFromName(const char * fontName)
    {
	    LOGFONTA lf;
	    memset(&lf, 0, sizeof(lf));
	    lf.lfQuality = CLEARTYPE_QUALITY;
	    lf.lfPitchAndFamily = DEFAULT_PITCH;
	    lf.lfItalic = false;
	    lf.lfHeight = 12;
	    lf.lfWidth = 0;
	    lf.lfWeight = FW_DONTCARE;
	    lf.lfCharSet = DEFAULT_CHARSET;
	    lf.lfUnderline = false;

	    for (unsigned int i = 0; i < strlen(fontName) && i < LF_FACESIZE - 1; i++)
		    lf.lfFaceName[i] = fontName[i];
	    // name should already be null terminated from the memset above
	    return lf;
    }

    virtual void renderText(const char * utf8, size_t length, RenderedLine * result)
    {
        // convert to utf16
        const WCHAR * utf16Text = NULL;
        size_t utf16Length = length; // TODO
        
        SCRIPT_ITEM * scriptItems = new SCRIPT_ITEM[utf16Length+1];
        int numItems = 0;
        HRESULT hr = ScriptItemize(utf16Text, utf16Length, utf16Length, NULL, NULL, scriptItems, &numItems);
        size_t maxGlyphs = 3 * length / 2 + 16;
        int numGlyphs = 0;
        WORD * pGlyphs = new WORD[maxGlyphs];
        WORD * pClusters = new WORD[utf16Length];
        SCRIPT_VISATTR * pVisAttr = new SCRIPT_VISATTR[maxGlyphs];
        for (int i = 0; i < numItems; i++)
        {
            ABC abc;
            hr = ScriptShape(m_hdc, m_script, utf16Text + scriptItems[i].iCharPos,
                scriptItems[i+1].iCharPos - scriptItems[i].iCharPos,
                maxGlyphs, &(scriptItems[i].a), pGlyphs, pClusters, pVisAttr, &numGlyphs);
            if (FAILED(hr))
                continue;
            GOFFSET * pOffsets = new GOFFSET[numGlyphs];
            int * pAdvances = new int[numGlyphs];
            hr = ScriptPlace(m_hdc, m_script, pGlyphs, numGlyphs, pVisAttr, &(scriptItems[i].a), pAdvances, pOffsets, &abc);
            if (FAILED(hr))
            {
                delete [] pOffsets;
                delete [] pAdvances;
                continue;
            }
            RenderedLine * renderedLine = new(result) RenderedLine(numGlyphs, static_cast<float>(pAdvances[numGlyphs]));
            for (int j = 0; j < numGlyphs; j++)
            {
                (*renderedLine)[j].set(pGlyphs[j], static_cast<float>(pOffsets[j].du),
                    static_cast<float>(pOffsets[j].dv), 0, 0);
            }
            delete [] pOffsets;
            delete [] pAdvances;
        }
        delete [] pGlyphs;
        delete [] pClusters;
        delete [] pVisAttr;
    }
    virtual const char * name() const { return "uniscribe"; }
private:
    int m_rtl;
    const char * m_fontFile;
	HDC m_hdc;
	LOGFONTA m_logFont;
	HFONT m_hFont;
	HFONT m_hFontOld;
	SCRIPT_CACHE *m_script;
};
