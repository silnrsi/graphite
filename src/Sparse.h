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
#pragma once

#include "Main.h"

namespace graphite2 {



class sparse
{
	typedef unsigned long	mask_t;

	static const unsigned char  SIZEOF_CHUNK = 48;

	struct chunk
	{
		mask_t			mask:SIZEOF_CHUNK;
		uint16			offset;
	};

public:
	typedef	uint16	key;
	typedef uint16	value;

	sparse() : m_nchunks(0) { m_array.map = 0; }
	~sparse() throw();

	template<typename I>
	sparse(I first, const I last);

	value 	operator [] (int k) const;
	operator bool () const { return m_array.map; }

	size_t capacity() const { return m_nchunks; }
	size_t size()     const;

	size_t _sizeof() const { return sizeof(sparse) + size()*sizeof(value) + m_nchunks*sizeof(chunk); }
private:
	union {
		chunk * map;
		value * values;
	}           m_array;
	key         m_nchunks;
};


template <typename I>
sparse::sparse(I attr, const I last)
: m_nchunks(0)
{
	// Find the maximum extent of the key space.
	size_t n_values=0;
	for (I i = attr; i != last; ++i, ++n_values)
	{
		const key k = i->id / SIZEOF_CHUNK;
		if (k >= m_nchunks) m_nchunks = k+1;
	}

	m_array.values = gralloc<value>((m_nchunks*sizeof(chunk) + sizeof(value)/2)/sizeof(value) + n_values*sizeof(value));

	if (!*this)
	{
		free(m_array.values); m_array.map=0;
		return;
	}

	chunk * ci = m_array.map;
	ci->mask   = 0;
	ci->offset = (m_nchunks*sizeof(chunk) + sizeof(value)-1)/sizeof(value);
	value * vi = m_array.values + ci->offset;
	for (key base = 0; attr != last; ++attr, ++vi)
	{
		const typename I::value_type v = *attr;
		const key chunks_diff = (v.id - base) / SIZEOF_CHUNK;

		if (chunks_diff)
		{
			ci   += chunks_diff;
			base += chunks_diff * SIZEOF_CHUNK;
			ci->offset = vi - m_array.values;
		}

		ci->mask |= 1UL << ((v.id - base) % SIZEOF_CHUNK);
		*vi = v.value;
	}
}

} // namespace graphite2
