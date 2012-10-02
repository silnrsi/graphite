/*-----------------------------------------------------------------------------
Copyright (C) 2011 SIL International
Responsibility: Tim Eves

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

Description:
The test harness for the Sparse class. This validates the
sparse classe is working correctly.
-----------------------------------------------------------------------------*/

#include <iostream>
#include <string>
#include "inc/Sparse.h"

using namespace graphite2;

inline
sparse::value_type v(sparse::key_type k)
{
    return sparse::value_type(k, sparse::mapped_type(~k));
}

namespace
{
    const sparse::value_type data[] =
    {
            v(0),
            v(2),   v(3),   v(5),   v(7),   v(11),  v(13),  v(17),  v(19),  v(23),  v(29),
            v(31),  v(37),  v(41),  v(43),  v(47),  v(53),  v(59),  v(61),  v(67),  v(71),
            v(73),  v(79),  v(83),  v(89),  v(97),  v(101), v(103), v(107), v(109), v(113),
            v(127), v(131), v(137), v(139), v(149), v(151), v(157), v(163), v(167), v(173),
            v(179), v(181), v(191), v(193), v(197), v(199), v(211), v(223), v(227), v(229),
            v(233), v(239), v(241), v(251), v(257), v(263), v(269), v(271), v(277), v(281),
            v(283), v(293), v(307), v(311), v(313), v(317), v(331), v(337), v(347), v(349),
            v(353), v(359), v(367), v(373), v(379), v(383), v(389), v(397), v(401), v(409),
            v(419), v(421), v(431), v(433), v(439), v(443), v(449), v(457), v(461), v(463),
            v(467), v(479), v(487), v(491), v(499), v(503), v(509), v(521), v(523), v(541)
    };
    const sparse::value_type * const data_end = data+sizeof(data)/sizeof(sparse::value_type);
}

int main(int argc , char *argv[])
{
    sparse sp(data, data_end);

    // Check all values are stored
    if (sp.size() != sizeof(data)/sizeof(sparse::value_type))
        return 1;

    // Check the values we put in are coming out again
    for (int i = 0; i != sizeof(data)/sizeof(sparse::value_type); ++i)
    {
        if (sp[data[i].first] != data[i].second)
            return 2;
    }

    // Check the "missing" values return 0
    const sparse::value_type * d = data;
    for (int i = 0; i != data_end[-1].first+1; ++i)
    {
        if (i == (*d).first)
        {
            if (sp[i] != (*d++).second)
                return 3;
        }
        else
        {
            if (sp[i] != 0)
                return 4;
        }
    }

    std::cout << "key range:\t" << data[0].first << "-" << data_end[-1].first << std::endl
              << "key space size: " << data_end[-1].first - data[0].first << std::endl
              << "linear uint16 array:" << std::endl
              << "\tcapacity:       " << data_end[-1].first+1 << std::endl
              << "\tresidency:      " << sizeof(data)/sizeof(sparse::value_type) << std::endl
              << "\tfill ratio:     " << 100.0f*(sizeof(data)/sizeof(sparse::value_type)/float(data_end[-1].first+1)) << "%" << std::endl
              << "\tsize:           " << (data_end[-1].first+1)*sizeof(uint16) << std::endl

              << "sparse uint16 array:" << std::endl
              << "\tcapacity:       " << sp.size() << std::endl
              << "\tresidency:      " << sp.size() << std::endl
              << "\tfill ratio:     " << 100.0f << "%" << std::endl
              << "\tsize:           " << sp._sizeof() << std::endl;

    // Check indexing and empty sparse array doesn't crash
    return 0;
}

