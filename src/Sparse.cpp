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
		v = v - ((v >> 1) & T(~T(0)/3));                           // temp
		v = (v & T(~T(0)/15*3)) + ((v >> 2) & T(~T(0)/15*3));      // temp
		v = (v + (v >> 4)) & T(~T(0)/255*15);                      // temp
		return (T)(v * T(~T(0)/255)) >> (sizeof(T)-1)*8;           // count
	}
}


sparse::~sparse() throw()
{
	free(m_array.values);
}


sparse::value sparse::operator [] (int k) const throw()
{
	const key			i = k/SIZEOF_CHUNK; k %= SIZEOF_CHUNK;
	const chunk & 		c = m_array.map[i];
	const mask_t 		m = c.mask >> (SIZEOF_CHUNK - 1 - k);

	return bool((m & 1)*(i < m_nchunks))*m_array.values[c.offset + bit_set_count(m >> 1)];
}


size_t sparse::size() const throw()
{
	size_t n = m_nchunks,
		   s = 0;

	for (const chunk *ci=m_array.map; n; --n, ++ci)
		s += bit_set_count(ci->mask);

	return s;
}
