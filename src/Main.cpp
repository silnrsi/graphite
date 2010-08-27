
// local definitions of new, delete
#include <cstdlib>
#include <cassert>
#include <cstdio>
#include "Main.h"

#ifdef __GNUC__

// define a few basic functions to save using libstdc++

extern "C" __attribute__ ((visibility("internal"))) void __cxa_pure_virtual()
{
    assert(false);
}

// declaration in c++/x.y/bits/functexcept.h
namespace std
{
    // it would be nice to remove these eventually
    // currently they are needed by ext/malloc_allocator.h and vector
    void __attribute__ ((visibility("internal"))) __throw_bad_alloc(void) { assert(false); };
    void __attribute__ ((visibility("internal"))) __throw_length_error(const char* c)
    { fprintf(stderr, "Length error %s\n", c); assert(false);}
}

#endif
