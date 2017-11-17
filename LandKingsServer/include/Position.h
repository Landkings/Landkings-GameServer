#pragma once
#include <cmath>

namespace Engine {

class Position {
public:
    Position(int x = 0, int y = 0);
    int getX() { return x; }
    int getY() { return y; }
    void setX(int nx) { x = nx; }
    void setY(int ny) { y = ny; }
    Position operator + (Position& right);
    Position operator - (Position& right);
    Position operator * (int val);
    double abs();
private:
    int x, y;
};

}
