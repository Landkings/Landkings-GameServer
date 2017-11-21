#include "Position.h"

using namespace Engine;

Position::Position(int x, int y) : x(x), y(y) {}

Position Position::operator+(const Position& right) {
    return Position(x + right.getX(), y + right.getY());
}

Position Position::operator-(const Position& right) {
    return Position(x - right.getX(), y - right.getY());
}

Position Engine::Position::operator*(int val) {
    return Position(x * val, y * val);
}

Position Position::operator =(const Position &right) {
    x = right.x;
    y = right.y;
    return *this;
}

double Position::abs() {
    return std::sqrt(x*x + y*y);
}
