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
#include <windows.h>
#include <usp10.h>
#include <wingdi.h>
#include <cstring>
#include <cwchar>

#include "Renderer.h"
#include "processUTF.h"
#include "TtfTypes.h"
#include "TtfUtil.h"

class UniscribeFunctions
{
public:
    typedef HRESULT (WINAPI *uspScriptItemize)
                (const WCHAR *pwcInChars,
                int cInChars, int cMaxItems, const SCRIPT_CONTROL *psControl,
                const SCRIPT_STATE *psState, SCRIPT_ITEM  *pItems, int *pcItems);
    typedef HRESULT (WINAPI *uspScriptShape)
                (HDC hdc, SCRIPT_CACHE *psc,
                const WCHAR *pwcChars, int cChars, int cMaxGlyphs, SCRIPT_ANALYSIS *psa,
                WORD *pwOutGlyphs, WORD *pwLogClust, SCRIPT_VISATTR *psva,
                int *pcGlyphs);
    typedef HRESULT (WINAPI *uspScriptPlace)
                (HDC hdc, SCRIPT_CACHE *psc, const WORD *pwGlyphs, int cGlyphs,
                const SCRIPT_VISATTR *psva, SCRIPT_ANALYSIS *psa, int *piAdvance,
                GOFFSET *pGoffset, ABC *pABC);

    UniscribeFunctions(const char * usp10dll)
        : fScriptPlace(NULL), fScriptShape(NULL), fScriptItemize(NULL), m_module(NULL)
    {
        m_module = LoadLibraryA(usp10dll);
        if (m_module)
        {
            fScriptItemize = reinterpret_cast<uspScriptItemize>
                (GetProcAddress(m_module, "ScriptItemize"));
            fScriptShape = reinterpret_cast<uspScriptShape>(GetProcAddress(m_module, "ScriptShape"));
            fScriptPlace = reinterpret_cast<uspScriptPlace>(GetProcAddress(m_module, "ScriptPlace"));
        }
        else
        {
            fprintf(stderr, "Library %s failed to load\n", usp10dll);
        }
    }
    ~UniscribeFunctions()
    {
        FreeLibrary(m_module);
    }
    uspScriptItemize fScriptItemize;
    uspScriptShape fScriptShape;
    uspScriptPlace fScriptPlace;
private:
    HMODULE m_module;
};

class UniscribeRenderer : public Renderer
{
public:
    UniscribeRenderer(const char * fontFile, const char * dll, int fontSize, int textDir)
        : m_usp(dll),
        m_rtl(textDir),
        m_fontFile(fontFile),
        m_hdc(GetDC(NULL)),
        m_hFont(NULL), m_hFontOld(NULL),
        m_glyphBufferSize(0),
        m_charBufferSize(0),
        m_script(NULL),
        m_scriptItems(NULL),
        m_pClusters(NULL),
        m_pVisAttr(NULL),
        m_pGlyphs(NULL),
        m_pOffsets(NULL),
        m_pAdvances(NULL),
        m_fontName(NULL)
    {
        m_logFont = createLogFontFromName(findFontName(fontFile), fontSize);
        if (AddFontResourceExA(fontFile, FR_PRIVATE, 0))
        {
            m_hFont = CreateFontIndirectW(&m_logFont);
            m_hFontOld = (HFONT) SelectObject(m_hdc, m_hFont);
        }
        else
        {
            fprintf(stderr, "AddFontResouce failed, try system font\n");
            m_hFont = CreateFontIndirectW(&m_logFont);
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
        delete [] m_scriptItems;
        delete [] m_pClusters;
        delete [] m_pVisAttr;
        delete [] m_pGlyphs;
        delete [] m_pOffsets;
        delete [] m_pAdvances;
        delete [] m_fontName;
        m_scriptItems = NULL;
        m_pClusters = NULL;
        m_pVisAttr = NULL;
        m_pGlyphs = NULL;
        m_pOffsets = NULL;
        m_pAdvances = NULL;
        m_fontName = NULL;
    }
    const WCHAR * findFontName(const char * fontFile)
    {
        FILE * pFile;
        m_fontName = NULL;
        if (!(pFile = fopen(fontFile, "rb"))) return NULL;
        size_t lOffset, lSize;
        if (!graphite2::TtfUtil::GetHeaderInfo(lOffset, lSize)) return NULL;
        char * pRawHeader = new char[lSize];
        graphite2::TtfUtil::Sfnt::OffsetSubTable * pHeader =
            reinterpret_cast<graphite2::TtfUtil::Sfnt::OffsetSubTable*>(pRawHeader);
        if (fseek(pFile, lOffset, SEEK_SET)) return NULL;
        if (fread(pRawHeader, 1, lSize, pFile) != lSize) return NULL;
        if (!graphite2::TtfUtil::CheckHeader(pHeader)) return NULL;
        if (!graphite2::TtfUtil::GetTableDirInfo(pHeader, lOffset, lSize)) return NULL;
        char * pRawTableDir = new char[lSize];
        graphite2::TtfUtil::Sfnt::OffsetSubTable::Entry* pTableDir =
            reinterpret_cast<graphite2::TtfUtil::Sfnt::OffsetSubTable::Entry*>(pRawTableDir);
        if (fseek(pFile, lOffset, SEEK_SET)) return NULL;
        if (fread(pRawTableDir, 1, lSize, pFile) != lSize) return NULL;
        if (!graphite2::TtfUtil::GetTableInfo(0x6e616d65, pHeader, pTableDir, lOffset, lSize)) return NULL;
        char * pRawNameTable = new char[lSize];
        if (fseek(pFile, lOffset, SEEK_SET)) return NULL;
        if (fread(pRawNameTable, 1, lSize, pFile) != lSize) return NULL;
        graphite2::TtfUtil::Sfnt::FontNames * pNames = 
            reinterpret_cast<graphite2::TtfUtil::Sfnt::FontNames *>(pRawNameTable);
        graphite2::TtfUtil::uint16 nameCount = (pNames->count >> 8) + ((pNames->count & 0xFF) << 8);
        if ((nameCount - 1) * sizeof(graphite2::TtfUtil::Sfnt::NameRecord) + 
            sizeof(graphite2::TtfUtil::Sfnt::FontNames) > lSize) return NULL;
        size_t nameOffset = 0;
        size_t nameSize = 0;
        if (graphite2::TtfUtil::GetNameInfo(pNames, 3, 1, 0x409, 4, nameOffset, nameSize))
        {
            assert(nameSize % 2 == 0);
            size_t utf16Len = nameSize/2;
            m_fontName = new WCHAR[utf16Len+1];
            memcpy(m_fontName, pRawNameTable + nameOffset, nameSize);
            graphite2::TtfUtil::SwapWString(m_fontName, utf16Len);
            m_fontName[utf16Len] = 0;
        }

        delete [] pRawHeader;
        delete [] pRawTableDir;
        delete [] pRawNameTable;
        fclose(pFile);
        return m_fontName;
    }
    LOGFONTW createLogFontFromName(const WCHAR * fontName, int size)
    {
        LOGFONTW lf;
        memset(&lf, 0, sizeof(lf));
        lf.lfQuality = CLEARTYPE_QUALITY;//ANTIALIASED_QUALITY;//CLEARTYPE_NATURAL_QUALITY;
        lf.lfPitchAndFamily = DEFAULT_PITCH;
        lf.lfItalic = false;
        int dpiY = GetDeviceCaps(m_hdc, LOGPIXELSY);
        lf.lfHeight = MulDiv(size, dpiY, 72);
        lf.lfWidth = 0;
        lf.lfWeight = FW_DONTCARE;
        lf.lfCharSet = DEFAULT_CHARSET;
        lf.lfUnderline = false;

        if (fontName)
        {
            for (unsigned int i = 0; i < wcslen(fontName) && i < LF_FACESIZE - 1; i++)
                lf.lfFaceName[i] = fontName[i];
        }
        // name should already be null terminated from the memset above
        return lf;
    }

    virtual void renderText(const char * utf8, size_t length, RenderedLine * result, FILE *log)
    {
        if (!length || !m_usp.fScriptItemize || !m_usp.fScriptShape || !m_usp.fScriptPlace)
        {
            new(result) RenderedLine(0, .0f);
            return;
        }
        // convert to utf16
        size_t utf16Length = length * 2 + 1;
        allocCharBuffers(utf16Length+1);
        assert(sizeof(WCHAR) == sizeof(gr_uint16));
        graphite2::ToUtf16Processor processor(reinterpret_cast<gr_uint16*>(m_utf16Text), m_charBufferSize);
        graphite2::IgnoreErrors ignore;
        graphite2::BufferLimit bufferLimit(gr_utf8, reinterpret_cast<const void*>(utf8), reinterpret_cast<const void*>(utf8 + length));
        graphite2::processUTF<graphite2::BufferLimit, graphite2::ToUtf16Processor, graphite2::IgnoreErrors>(bufferLimit, &processor, &ignore);
        assert(utf16Length > processor.uint16Processed());
        utf16Length = processor.uint16Processed();
        m_utf16Text[utf16Length] = 0;
        int numItems = 0;
        int offset = 0;
        if (m_utf16Text[0] == 0xFEFF)
            offset += 1;
        // maxItems must be at least 2 otherwise scriptitemize fails
        HRESULT hr = (m_usp.fScriptItemize)(m_utf16Text + offset, utf16Length - offset, m_charBufferSize, NULL, NULL, m_scriptItems, &numItems);
        if (FAILED(hr))
        {
            new(result) RenderedLine(0, .0f);
            return;
        }
        size_t maxGlyphs = 3 * length / 2 + 16;
        allocGlyphBuffers(maxGlyphs);
        int numGlyphs = 0;
        int glyphOffset = 0;
        int advance = 0;
        for (int i = 0; i < numItems; i++)
        {
            ABC abc;
            int itemLength = (i + 1 < numItems)? (m_scriptItems[i+1].iCharPos - m_scriptItems[i].iCharPos) :
                (utf16Length - m_scriptItems[i].iCharPos);
            hr = (m_usp.fScriptShape)(m_hdc, &m_script, m_utf16Text + m_scriptItems[i].iCharPos,
                itemLength, maxGlyphs - glyphOffset, &(m_scriptItems[i].a),
                m_pGlyphs + glyphOffset, m_pClusters + m_scriptItems[i].iCharPos,
                m_pVisAttr + glyphOffset, &numGlyphs);
            if (hr == E_OUTOFMEMORY)
            {
                allocGlyphBuffers(2 * maxGlyphs);
                renderText(utf8, length, result);
                return;
            }
            if (FAILED(hr))
                continue;
            hr = (m_usp.fScriptPlace)(m_hdc, &m_script, m_pGlyphs + glyphOffset, numGlyphs,
                m_pVisAttr + glyphOffset, &(m_scriptItems[i].a), m_pAdvances + glyphOffset,
                m_pOffsets + glyphOffset, &abc);
            if (FAILED(hr))
            {
                continue;
            }
            glyphOffset += numGlyphs;
            advance += abc.abcB + abc.abcA + abc.abcC;
        }
        RenderedLine * renderedLine = new(result) RenderedLine(glyphOffset, static_cast<float>(advance));
        int cumulativeAdvance = 0;
        int firstChar = -1;
        int lastChar = -1;
        int cluster = 0;
        for (int j = 0; j < glyphOffset; j++)
        {
            if (m_pVisAttr[j].fClusterStart)
            {
                firstChar = lastChar + 1;
                lastChar = firstChar;
                while ((lastChar + 1 < static_cast<int>(utf16Length)) && (m_pClusters[lastChar+1] == cluster))
                {
                    ++lastChar;
                }
                ++cluster;
            }
            float x = static_cast<float>(m_pOffsets[j].du + cumulativeAdvance);
            float y = static_cast<float>(m_pOffsets[j].dv);
            (*renderedLine)[j].set(m_pGlyphs[j], x, y, firstChar, lastChar);
            cumulativeAdvance += m_pAdvances[j];
        }
        renderedLine->setAdvance(static_cast<float>(cumulativeAdvance));
    }
    virtual const char * name() const { return "uniscribe"; }
private:
    bool allocGlyphBuffers(size_t maxSize)
    {
        if (maxSize > m_glyphBufferSize)
        {
            delete [] m_pVisAttr;
            delete [] m_pGlyphs;
            delete [] m_pOffsets;
            delete [] m_pAdvances;
            m_pVisAttr = new SCRIPT_VISATTR[maxSize];
            m_pGlyphs = new WORD[maxSize];
            m_pOffsets = new GOFFSET[maxSize];
            m_pAdvances = new int[maxSize];
            m_glyphBufferSize = maxSize;
        }
        return (m_pVisAttr && m_pGlyphs && m_pOffsets && m_pAdvances && m_glyphBufferSize);
    }
    bool allocCharBuffers(size_t maxSize)
    {
        if (maxSize > m_charBufferSize)
        {
            delete [] m_pClusters;
            delete [] m_scriptItems;
            delete [] m_utf16Text;
            m_scriptItems  = new SCRIPT_ITEM[maxSize];
            m_pClusters = new WORD[maxSize];
            m_utf16Text = new WCHAR[maxSize];
            m_charBufferSize = maxSize;
        }
        return (m_scriptItems && m_pClusters);
    }

    UniscribeFunctions m_usp;
    int m_rtl;
    const char * m_fontFile;
    HDC m_hdc;
    LOGFONTW m_logFont;
    HFONT m_hFont;
    HFONT m_hFontOld;
    size_t m_glyphBufferSize;
    size_t m_charBufferSize;
    SCRIPT_CACHE m_script;
    SCRIPT_ITEM * m_scriptItems;
    WCHAR * m_utf16Text;
    WORD * m_pClusters;
    SCRIPT_VISATTR * m_pVisAttr;
    WORD * m_pGlyphs;
    GOFFSET * m_pOffsets;
    int * m_pAdvances;
    WCHAR * m_fontName;
};
