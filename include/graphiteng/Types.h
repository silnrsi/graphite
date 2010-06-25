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
#define GRNG_EXPORT
#endif

#define FIND_BROKEN_VIRTUALS



class FontImpl;

class GRNG_EXPORT Position
{
public:

    Position() : x(0), y(0) { }
    Position(const Position &other) {x = other.x, y = other.y;}
    Position(float inx, float iny) { x = inx; y = iny; }
    Position operator + (Position a) { return Position(x + a.x, y + a.y); }
    Position &operator += (const Position &a) { x += a.x; y += a.y; return *this; }

    float x;
    float y;
};

class GRNG_EXPORT Rect
{
public :

    Rect() {}
    Rect(Position origin, Position extent) { bl = origin; tr = extent; }
    Rect(const Rect &other) {bl = other.bl; tr = other.tr;}
    Rect widen(Rect other) { return Rect(Position(bl.x > other.bl.x ? other.bl.x : bl.x, bl.y > other.bl.y ? other.bl.y : bl.y), Position(tr.x > other.tr.x ? tr.x : other.tr.x, tr.y > other.tr.y ? tr.y : other.tr.y)); }
    Rect operator + (const Position &a) const { return Rect(Position(bl.x + a.x, bl.y + a.y), Position(tr.x + a.x, tr.y + a.y)); }

    Position bl;
    Position tr;
};

#endif // GRTYPES_INCLUDE
