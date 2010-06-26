#ifndef GRTYPES_INCLUDE
#define GRTYPES_INCLUDE

#include <cstddef>

typedef unsigned char uint8;
typedef uint8    byte;
typedef signed char int8;
typedef unsigned short uint16;
typedef short   int16;
typedef unsigned int    uint32;
typedef int     int32;

#ifdef _MSC_VER
#define GRNG_EXPORT __declspec(dllexport)
#else
#ifdef __GNUC__
#define GRNG_EXPORT __attribute__ ((visibility("default")))
#else
#define GRNG_EXPORT
#endif
#endif

#define FIND_BROKEN_VIRTUALS




class Position
{
public:

    Position() : x(0), y(0) { }
    Position(float inx, float iny) { x = inx; y = iny; }
    Position operator + (const Position& a) const { return Position(x + a.x, y + a.y); }
    Position &operator += (const Position &a) { x += a.x; y += a.y; return *this; }

    float x;
    float y;
};

#endif // GRTYPES_INCLUDE
