#pragma once

namespace org { namespace sil { namespace graphite { namespace v2 {

class DoNotInitialize
{
};
  
class Position
{
public:
    Position(const DoNotInitialize&) {}
    Position() : x(0), y(0) { }
    Position(float inx, float iny) { x = inx; y = iny; }
    Position operator + (const Position& a) const { return Position(x + a.x, y + a.y); }
    Position operator - (const Position& a) const { return Position(x - a.x, y - a.y); }
    Position operator * (const float m) const { return Position(x * m, y * m); }
    Position &operator += (const Position &a) { x += a.x; y += a.y; return *this; }

    float x;
    float y;
};

}}}} // namespace
