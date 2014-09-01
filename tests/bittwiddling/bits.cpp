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

#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include "inc/Main.h"
#include "inc/bits.h"

using namespace graphite2;

#define v(n) (~(-1LL << n))
#define b8(i) 	v(i+0), v(i+1), v(i+2), v(i+3), v(i+4), v(i+5), v(i+6), v(i+7), v(i+8)
#define b16(i) 	b8(i+0), b8(i+9)
#define b32(i) 	b16(i+0), b16(i+18)
#define b64(i)  b32(i+0), b32(i+36)

//#define BENCHMARK 40000000
#if defined BENCHMARK
#define benchmark() for (int n = BENCHMARK; n; --n)
#else
#define benchmark()
#endif


typedef     unsigned long   uint64;
typedef     signed long     int64;
namespace
{
	uint8 	u8_pat[] = { b8(0) };
	int8 	s8_pat[] = { b8(0) };
	uint16 	u16_pat[] = { b16(0) };
	int16 	s16_pat[] = { b16(0) };
	uint32 	u32_pat[] = { b32(0) };
	int32 	s32_pat[] = { b32(0) };
	uint64 	u64_pat[] = { b64(0) };
	int64 	s64_pat[] = { b64(0) };

    int ret = 0;

    
    template<typename T> 
    struct type_name {};
    
    template<typename T>
    inline 
    std::ostream & operator << (std::ostream & o, type_name<T>)
    {
        if (!std::numeric_limits<T>::is_signed) o.put('u');
        o << "int" << std::dec << sizeof(T)*8;
    }
    
	template<typename T>
	inline
	void test_bit_set_count(const T pat[])
	{
		for (unsigned int p = 0; p <= sizeof(T)*8; ++p)
		{
#if !defined BENCHMARK
			std::cout << "bit_set_count("
			                << (!std::numeric_limits<T>::is_signed ? "uint" : "int")
			                << std::dec << sizeof(T)*8 << "(0x"
			                    << std::hex 
			                    << std::setw(sizeof(T)*2) 
			                    << std::setfill('0') 
			                    << static_cast<unsigned long>(pat[p]) 
		                << ")) -> "  
		                    << std::dec 
	                        <<  bit_set_count(pat[p]) << std::endl;
#endif
			if (bit_set_count(pat[p]) != p)
			{
				std::cerr << " != " << std::dec << p << std::endl;
			    ret = sizeof(T);
			}
		}
	}

}

int main(int argc , char *argv[])
{
    benchmark()
    {
        // Test bit_set_count
        test_bit_set_count(u8_pat);
        test_bit_set_count(s8_pat);
        test_bit_set_count(u16_pat);
        test_bit_set_count(s16_pat);
        test_bit_set_count(u32_pat);
        test_bit_set_count(s32_pat);
        test_bit_set_count(u64_pat);
        test_bit_set_count(s64_pat);
    }
    
    return ret;
}

