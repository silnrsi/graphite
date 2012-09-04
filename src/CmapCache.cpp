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

Alternatively, the contents of this file may be used under the terms of the
Mozilla Public License (http://mozilla.org/MPL) or the GNU General Public
License, as published by the Free Software Foundation, either version 2
of the License or (at your option) any later version.
*/

#include "inc/Main.h"
#include "inc/CmapCache.h"
#include "inc/Face.h"
#include "inc/TtfTypes.h"
#include "inc/TtfUtil.h"


using namespace graphite2;

CachedCmap::CachedCmap(const Face & face)
: m_isBmpOnly(true),
  m_blocks(0)
{
    const Face::Table cmap(face, Tag::cmap);
    if (!cmap)	return;

    const void * table31 = TtfUtil::FindCmapSubtable(cmap, 3, 1, cmap.size());
    const void * table310 = TtfUtil::FindCmapSubtable(cmap, 3, 10, cmap.size());
    m_isBmpOnly = (!table310);
    int rangeKey = 0;
    uint32 	codePoint = 0,
    		prevCodePoint = 0;
    if (table310 && TtfUtil::CheckCmap310Subtable(table310))
    {
        m_blocks = grzeroalloc<uint16*>(0x1100);
        if (!m_blocks) return;
        codePoint =  TtfUtil::Cmap310NextCodepoint(table310, codePoint, &rangeKey);
        while (codePoint != 0x10FFFF)
        {
            unsigned int block = (codePoint & 0xFFFF00) >> 8;
            if (!m_blocks[block])
            {
                m_blocks[block] = grzeroalloc<uint16>(0x100);
                if (!m_blocks[block])
                    return;
            }
            m_blocks[block][codePoint & 0xFF] = TtfUtil::Cmap310Lookup(table310, codePoint, rangeKey);
            // prevent infinite loop
            if (codePoint <= prevCodePoint)
                codePoint = prevCodePoint + 1;
            prevCodePoint = codePoint;
            codePoint =  TtfUtil::Cmap310NextCodepoint(table310, codePoint, &rangeKey);
        }
    }
    else
    {
        m_blocks = grzeroalloc<uint16*>(0x100);
        if (!m_blocks) return;
    }
    if (table31 && TtfUtil::CheckCmap31Subtable(table31))
    {
        codePoint = 0;
        rangeKey = 0;
        codePoint =  TtfUtil::Cmap31NextCodepoint(table31, codePoint, &rangeKey);
        while (codePoint != 0xFFFF)
        {
            unsigned int block = (codePoint & 0xFFFF00) >> 8;
            if (!m_blocks[block])
            {
                m_blocks[block] = grzeroalloc<uint16>(0x100);
                if (!m_blocks[block])
                    return;
            }
            m_blocks[block][codePoint & 0xFF] = TtfUtil::Cmap31Lookup(table31, codePoint, rangeKey);
            // prevent infinite loop
            if (codePoint <= prevCodePoint)
                codePoint = prevCodePoint + 1;
            prevCodePoint = codePoint;
            codePoint =  TtfUtil::Cmap31NextCodepoint(table31, codePoint, &rangeKey);
        }
    }
}

CachedCmap::~CachedCmap() throw()
{
    unsigned int numBlocks = (m_isBmpOnly)? 0x100 : 0x1100;
    for (unsigned int i = 0; i < numBlocks; i++)
    	free(m_blocks[i]);
    free(m_blocks);
}

uint16 CachedCmap::operator [] (const uint32 usv) const throw()
{
    if ((m_isBmpOnly && usv > 0xFFFF) || (usv > 0x10FFFF))
        return 0;
    const uint32 block = 0xFFFF & (usv >> 8);
    if (m_blocks[block])
        return m_blocks[block][usv & 0xFF];
    return 0;
};

CachedCmap::operator bool() const throw()
{
	return m_blocks != 0;
}


DirectCmap::DirectCmap(const Face & face)
: _cmap(face, Tag::cmap),
  _stable(0),
  _ctable(0)
{
    if (!_cmap)	return;

    _ctable = TtfUtil::FindCmapSubtable(_cmap, 3, 1, _cmap.size());
    if (!_ctable || !TtfUtil::CheckCmap31Subtable(_ctable))
    {
        _ctable =  0;
        return;
    }
    _stable = TtfUtil::FindCmapSubtable(_cmap, 3, 10, _cmap.size());
    if (_stable && !TtfUtil::CheckCmap310Subtable(_stable))
    	_stable = 0;
}

uint16 DirectCmap::operator [] (const uint32 usv) const throw()
{
    return usv > 0xFFFF ? (_stable ? TtfUtil::Cmap310Lookup(_stable, usv) : 0) : TtfUtil::Cmap31Lookup(_ctable, usv);
}

DirectCmap::operator bool () const throw()
{
	return _cmap && _ctable;
}

