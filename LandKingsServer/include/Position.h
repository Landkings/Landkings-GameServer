#pragma once
#include <cmath>
#include <iostream>

#include "lua.hpp"
#include "LuaHelper.h"

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
    void luaPush(lua_State *state);
private:
    int getX(lua_State *state);
    int getY(lua_State *state);
    int x, y;
};

}
