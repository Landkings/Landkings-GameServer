#pragma once
#include <cmath>

class Point {
public:
    Point(int x = 0, int y = 0);
    int getX();
    int getY();
    void setX(int nx);
    void setY(int ny);
    Point operator + (Point right);
    Point operator - (Point right);
    double abs();
private:
    int x, y;
};