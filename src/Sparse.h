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
	static const mask_t         CHUNK_BITS   = mask_t(1UL << (SIZEOF_CHUNK - 1)) | mask_t((1UL << (SIZEOF_CHUNK - 1)) - 1);

	struct chunk
	{
		mask_t			mask:SIZEOF_CHUNK;
		uint16			offset;
	};

public:
	typedef	uint16	key;
	typedef uint16	value;

	sparse() : m_limit(0), m_map(0), m_values(0) {}
	~sparse() throw();

	template<typename I>
	sparse(I first, const I last);

	value 	operator [] (int k) const;
	operator bool () const { return m_map && m_values; }

	size_t capacity() const { return m_limit; }
	size_t size()     const;

//	size_t _sizeof() const { return sizeof(sparse) + size()*sizeof(value) + (m_limit + SIZEOF_CHUNK-1)/SIZEOF_CHUNK*sizeof(chunk); }
private:
	chunk & get_chunk(key & k);
	const chunk & get_chunk(key & k) const { return const_cast<sparse *>(this)->get_chunk(k); }

	key		m_limit;
	chunk *	m_map;
	value *	m_values;
};


template <typename I>
sparse::sparse(I attr, const I last)
: m_limit(0), m_map(0), m_values(0)
{
	// Find the maximum extent of the key space.
	size_t n_values=0;
	for (I i = attr; i != last; ++i, ++n_values)
	{
		const key k = i->id;
		if (k >= m_limit) m_limit = k+1;
	}

	m_map    = grzeroalloc<chunk>((m_limit + SIZEOF_CHUNK-1)/SIZEOF_CHUNK);
	m_values = gralloc<value>(n_values);

	if (!*this)
	{
		free(m_map); free(m_values);
		m_map = 0; m_values = 0;
		return;
	}

	chunk * ci = m_map;
	value * vi = m_values;
	for (key base = 0; attr != last; ++attr, ++vi)
	{
		const typename I::value_type v = *attr;
		const key chunks_diff = (v.id - base) / SIZEOF_CHUNK;

		if (chunks_diff)
		{
			ci   += chunks_diff;
			base += chunks_diff * SIZEOF_CHUNK;
			ci->offset = vi - m_values;
		}

		ci->mask |= 1UL << (SIZEOF_CHUNK - 1 - ((v.id - base) % SIZEOF_CHUNK));
		*vi = v.value;
	}
}

} // namespace graphite2
