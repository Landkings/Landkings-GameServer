#pragma once
#include <cmath>

namespace Engine {

class Position {
public:
    Position(int x = 0, int y = 0);
    int getX() const { return x; }
    int getY() const { return y; }
    void setX(const int nx) { x = nx; }
    void setY(const int ny) { y = ny; }
    Position operator + (const Position& right);
    Position operator - (const Position& right);
    Position operator * (int val);
    Position operator = (const Position& right);
    double abs();
private:
    int x, y;
};

}
