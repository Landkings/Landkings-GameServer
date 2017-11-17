#include "Position.h"

using namespace Engine;

Position::Position(int x, int y) : x(x), y(y) {}

Position Position::operator+(Position& right) {
    return Position(x + right.getX(), y + right.getY());
}

Position Position::operator-(Position& right) {
    return Position(x - right.getX(), y - right.getY());
}

Position Engine::Position::operator*(int val) {
    return Position(x*val, y*val);
}

double Position::abs() {
    return std::sqrt(x*x + y*y);
}
