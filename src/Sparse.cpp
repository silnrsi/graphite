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
    If not, write to the Free Software Foundation, Inc., 59 Temple Place,
    Suite 330, Boston, MA 02111-1307, USA or visit their web page on the
    internet at http://www.fsf.org/licenses/lgpl.html.

Alternatively, the contents of this file may be used under the terms of the
Mozilla Public License (http://mozilla.org/MPL) or the GNU General Public
License, as published by the Free Software Foundation, either version 2
of the License or (at your option) any later version.
*/

#include "Sparse.h"

using namespace graphite2;

namespace
{

	template<typename T>
	inline unsigned int bit_set_count(T v)
	{
		v = v - ((v >> 1) & (T)~(T)0/3);                           // temp
		v = (v & (T)~(T)0/15*3) + ((v >> 2) & (T)~(T)0/15*3);      // temp
		v = (v + (v >> 4)) & (T)~(T)0/255*15;                      // temp
		const unsigned int c = (T)(v * ((T)~(T)0/255)) >> (sizeof(T)-1)*8; // count
		return c;
	}

	inline unsigned int zero_trailing_count(uint32 v)
	{
		static const unsigned int MultiplyDeBruijnBitPosition[32] =
		{
		  0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8,
		  31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
		};

		return MultiplyDeBruijnBitPosition[(uint32((v & -v) * 0x077CB531U)) >> 27];
	}

	inline unsigned int log2(uint32 v)
	{
		static const unsigned int MultiplyDeBruijnBitPosition[32] =
		{
		  0, 9, 1, 10, 13, 21, 2, 29, 11, 14, 16, 18, 22, 25, 3, 30,
		  8, 12, 20, 28, 15, 17, 24, 7, 19, 27, 23, 6, 26, 5, 4, 31
		};

		v |= v >> 1; // first round down to one less than a power of 2
		v |= v >> 2;
		v |= v >> 4;
		v |= v >> 8;
		v |= v >> 16;

		return MultiplyDeBruijnBitPosition[uint32((v * 0x07C4ACDDU) >> 27)];
	}
}


sparse::~sparse() throw()
{
	free(m_map);
	free(m_values);
}


sparse::value sparse::operator [] (int k) const
{
	const chunk & 		c = m_map[k/SIZEOF_CHUNK];
	const unsigned int	o = k % SIZEOF_CHUNK;
	const mask_t 		b = 1UL << (SIZEOF_CHUNK-1 - o);
	const unsigned int  bs = bit_set_count((c.mask | ((b << 1) - 1)) ^ CHUNK_BITS);

	return bool((c.mask & b)*(k < m_limit))*m_values[c.offset + o - bs];
}


size_t sparse::size() const
{
	size_t n = (m_limit + SIZEOF_CHUNK-1)/SIZEOF_CHUNK,
		   s = 0;

	for (const chunk *ci=m_map; n; --n, ++ci)
		s += bit_set_count(ci->mask);

	return s;
}
