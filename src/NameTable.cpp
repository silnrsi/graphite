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
#include "Main.h"

#include "NameTable.h"
#include "processUTF.h"

namespace org { namespace sil { namespace graphite { namespace v2 {

NameTable::NameTable(const void* data, size_t length, uint16 platformId, uint16 encodingID)
    : m_table(reinterpret_cast<const TtfUtil::Sfnt::FontNames*>(data)),
    m_platformOffset(0), m_nameDataLength(0), m_nameData(NULL)
{
    if ((length > sizeof(TtfUtil::Sfnt::FontNames)) &&
        (length > sizeof(TtfUtil::Sfnt::FontNames) +
         sizeof(TtfUtil::Sfnt::NameRecord) * ( swap16(m_table->count) - 1)))
    {
        uint16 offset = swap16(m_table->string_offset);
        m_nameData = reinterpret_cast<const uint8*>(data) + offset;
        setPlatformEncoding(platformId, encodingID);
        m_nameDataLength = length - offset;
    }
    else
    {
        m_table = NULL;
    }
}

uint16 NameTable::setPlatformEncoding(uint16 platformId, uint16 encodingID)
{
    if (!m_nameData) return 0;
    uint16 i = 0;
    for (; i < m_table->count; i++)
    {
        if (swap16(m_table->name_record[i].platform_id) == platformId &&
            swap16(m_table->name_record[i].platform_specific_id) == encodingID)
        {
            m_platformOffset = i;
            break;
        }
    }
    while ((++i < m_table->count) &&
           (swap16(m_table->name_record[i].platform_id) == platformId) &&
           (swap16(m_table->name_record[i].platform_specific_id) == encodingID))
    {
        m_platformLastRecord = i;
    }
    return 0;
}

void* NameTable::getName(uint16& languageId, uint16 nameId, gr_encform enc, uint32& length)
{
    uint16 anyLang = 0;
    uint16 enUSLang = 0;
    uint16 similarLang = 0;
    uint16 actualLang = 0;
    uint16 bestLang = 0;
    if (!m_table)
    {
        languageId = 0;
        length = 0;
        return NULL;
    }
    for (uint16 i = m_platformOffset; i <= m_platformLastRecord; i++)
    {
        if (swap16(m_table->name_record[i].name_id) == nameId)
        {
            uint16 langId = swap16(m_table->name_record[i].language_id);
            if (langId == languageId)
            {
                actualLang = i;
                bestLang = i;
                break;
            }
            // MS language tags have the language in the lower byte, region in the higher
            else if ((langId & 0xFF) == (languageId & 0xFF))
            {
                similarLang = i;
                bestLang = i;
            }
            else if (langId == 0x409)
            {
                enUSLang = i;
            }
            else
            {
                anyLang = i;
            }
        }
    }
    if (!bestLang)
    {
        if (enUSLang) bestLang = enUSLang;
        else
        {
            bestLang = anyLang;
            if (!anyLang)
            {
                languageId = 0;
                length = 0;
                return NULL;
            }
        }
    }
    const TtfUtil::Sfnt::NameRecord & nameRecord = m_table->name_record[bestLang];
    languageId = swap16(nameRecord.language_id);
    uint16 utf16Length = swap16(nameRecord.length);
    uint16 offset = swap16(nameRecord.offset);
    if(offset + utf16Length > m_nameDataLength)
    {
        languageId = 0;
        length = 0;
        return NULL;
    }
    utf16Length >>= 1; // in utf16 units
    uint16 * utf16Name = gralloc<uint16>(utf16Length + 1);
    const uint8* pName = m_nameData + offset;
    for (size_t i = 0; i < utf16Length; i++)
    {
        utf16Name[i] = read16(pName);
    }
    utf16Name[utf16Length] = 0;
    if (enc == gr_utf16)
    {
        length = utf16Length;
        return utf16Name;
    }
    else if (enc == gr_utf8)
    {
        uint8* uniBuffer = gralloc<uint8>(3 * utf16Length + 1);
        ToUtf8Processor processor(uniBuffer, 3 * utf16Length + 1);
        IgnoreErrors ignore;
        BufferLimit bufferLimit(gr_utf16, reinterpret_cast<void*>(utf16Name), reinterpret_cast<void*>(utf16Name + utf16Length));
        processUTF<BufferLimit, ToUtf8Processor, IgnoreErrors>(bufferLimit, &processor, &ignore);
        length = processor.bytesProcessed();
        uniBuffer[processor.bytesProcessed()] = 0;
        free(utf16Name);
        return uniBuffer;
    }
    else if (enc == gr_utf32)
    {
        uint32 * uniBuffer = gralloc<uint32>(utf16Length  + 1);
        IgnoreErrors ignore;
        BufferLimit bufferLimit(gr_utf16, reinterpret_cast<void*>(utf16Name), reinterpret_cast<void*>(utf16Name + utf16Length));

        ToUtf32Processor processor(uniBuffer, utf16Length);
        processUTF(bufferLimit, &processor, &ignore);
        length = processor.charsProcessed();
        uniBuffer[length] = 0;
        free(utf16Name);
        return uniBuffer;
    }
    length = 0;
    return NULL;
}

uint16 NameTable::getLanguageId(const char * bcp47Locale)
{
    size_t localeLength = strlen(bcp47Locale);
    uint16 localeId = m_locale2Lang.getMsId(bcp47Locale);
    if (m_table && (swap16(m_table->format) == 1))
    {
        const uint8 * pLangEntries = reinterpret_cast<const uint8*>(m_table) +
            sizeof(TtfUtil::Sfnt::FontNames)
            + sizeof(TtfUtil::Sfnt::NameRecord) * ( swap16(m_table->count) - 1);
        uint16 numLangEntries = read16(pLangEntries);
        const TtfUtil::Sfnt::LangTagRecord * langTag =
            reinterpret_cast<const TtfUtil::Sfnt::LangTagRecord*>(pLangEntries);
        if (pLangEntries + numLangEntries * sizeof(TtfUtil::Sfnt::LangTagRecord) <= m_nameData)
        {
            for (uint16 i = 0; i < numLangEntries; i++)
            {
                uint16 offset = swap16(langTag[i].offset);
                uint16 length = swap16(langTag[i].length);
                if ((offset + length <= m_nameDataLength) && (length == 2 * localeLength))
                {
                    const uint8* pName = m_nameData + offset;
                    bool match = true;
                    for (size_t j = 0; j < localeLength; j++)
                    {
                        uint16 code = read16(pName);
                        if ((code > 0x7F) || (code != bcp47Locale[j]))
                        {
                            match = false;
                            break;
                        }
                    }
                    if (match)
                        return 0x8000 + i;
                }
            }
        }
    }
    return localeId;
}

}}}}
