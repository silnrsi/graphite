
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

void __attribute__ ((visibility("internal"))) *operator new (std::size_t size) throw() { return malloc(size); }
// void __attribute__ ((visibility("internal"))) *operator new (std::size_t size, const std::nothrow_t& nothrow_constant) throw() { return malloc(size); }
void *operator new (std::size_t size, void *ptr) throw() { return ptr; }

void __attribute__ ((visibility("internal"))) operator delete (void* ptr) throw () { free(ptr); }
// void __attribute__ ((visibility("internal"))) operator delete (void* ptr, const std::nothrow_t& nothrow_constant) throw() { free(ptr); }
void operator delete (void* ptr, void* voidptr2) throw() { }

void* __attribute__ ((visibility("internal"))) operator new[] (std::size_t size) throw () { return malloc(size); }
// void* __attribute__ ((visibility("internal"))) operator new[] (std::size_t size, const std::nothrow_t& nothrow_constant) throw() { return malloc(size); }
void* operator new[] (std::size_t size, void* ptr) throw() { return ptr; }

void __attribute__ ((visibility("internal"))) operator delete[] (void* ptr) throw () { free(ptr); }
// void __attribute__ ((visibility("internal"))) operator delete[] (void* ptr, const std::nothrow_t& nothrow_constant) throw() { free(ptr); }
void operator delete[] (void* ptr, void* voidptr2) throw() { }
