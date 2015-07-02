/*  Copyright (c) 2012, Siyuan Fu <fusiyuan2010@gmail.com>
    Portions Copyright (c) 2015, SIL International
    All rights reserved.
    
    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    
    1. Redistributions of source code must retain the above copyright notice,
       this list of conditions and the following disclaimer.
    
    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
    
    3. Neither the name of the copyright holder nor the names of its
       contributors may be used to endorse or promote products derived from
       this software without specific prior written permission.
    
    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
    POSSIBILITY OF SUCH DAMAGE.
*/
#include <cassert>

#include "inc/Decompressor.h"
#include "inc/Shrinker.h"

using namespace shrinker;

struct match_op
{
    u8 match_len:4;
    u8 long_dist:1;
    u8 literal_len:3;
};

inline
void read_literal(u32 & v, u8 const * &s) {
    u8 b = 0; 
    do { v += b = *s++; } while(b==0xff);
}

u8 const LONG_DIST = 0x10;
u8 const MATCH_LEN = 0x0f;

int shrinker::decompress(void const *in, void *out, size_t size)
{
    u8 const * src = static_cast<u8 const *>(in);
    u8 * dst = static_cast<u8*>(out);
    u8 * const end = dst + size;
    
    while (true) 
    {
        u8 const flag = *src++;
        
        u32 literal_len = flag >> 5;
        if (unlikely(literal_len == 7))
            read_literal(literal_len, src);
        
        u32 match_len = flag & MATCH_LEN;
        if (unlikely(match_len == 15))
            read_literal(match_len, src);
        
        u32 match_dist = *src++;
        if (flag & LONG_DIST) 
        {
            match_dist |= ((*src++) << 8);
            if (unlikely(match_dist == 0xffff)) {
                if (unlikely(dst + literal_len > end)) return -1;
                memcpy_nooverlap_surpass(dst, src, literal_len);
                break;
            }
        }
        
        // Copy in literal
        if (unlikely(dst + literal_len > end)) return -1;
        memcpy_nooverlap(dst, src, literal_len);
        src += literal_len;
        
        // Copy, possibly repeating, match from earlier in the
        //  decoded output.
        u8 const * const pcpy = dst - match_dist - 1;
        if (unlikely(pcpy < static_cast<u8*>(out) 
                  || dst + match_len + MINMATCH > end)) return -1;
        memcpy_(dst, pcpy, match_len + MINMATCH);
    }
    return dst - (u8*)out;
}

