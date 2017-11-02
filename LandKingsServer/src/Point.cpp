#include "Point.h"

Point::Point(int x, int y) : x(x), y(y) {}

int Point::getX() {
    return x;
}

int Point::getY() {
    return y;
}

void Point::setX(int nx) {
    x = nx;
}

void Point::setY(int ny) {
    y = ny;
}

Point Point::operator+(Point right) {
    return Point(x + right.getX(), y + right.getY());
}

Point Point::operator-(Point right) {
    return Point(x - right.getX(), y - right.getY());
}

double Point::abs() {
    return std::sqrt(x*x + y*y);
}
