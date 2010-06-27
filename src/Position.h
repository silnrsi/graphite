#ifndef POSITION_INCLUDE
#define POSITION_INCLUDE


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

#endif
