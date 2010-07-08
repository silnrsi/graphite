
// local definitions of new, delete
#include <cstdlib>
#include <cassert>
#include <cstdio>

#ifdef __GNUC__

// define a few basic functions to save using libstdc++

extern "C" __attribute__ ((visibility("internal"))) void __cxa_pure_virtual()
{
    assert(false);
}

// visibility is hard coded in the compiler for new and delete operators

void * operator new[](size_t size)
{
    return malloc(size);
}

void operator delete[] (void * p)
{
    if (p) free(p);
}

void * operator new(size_t size)
{
    return malloc(size);
}

void operator delete (void * p)
{
    if (p) free(p);
}

// declaration in c++/x.y/bits/functexcept.h
namespace std
{
    // it would be nice to remove these eventually
    void __attribute__ ((visibility("internal"))) __throw_bad_alloc(void) { assert(false); };
    void __attribute__ ((visibility("internal"))) __throw_length_error(const char* c)
    { fprintf(stderr, "Length error %s\n", c); assert(false);}
}

#endif
