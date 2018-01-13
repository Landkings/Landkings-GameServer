#pragma once
#include <cmath>
#include <iostream>

#include "lua.hpp"
#include "LuaHelper.h"

namespace Engine {

class Vec2i {
public:
    Vec2i(int x = 0, int y = 0);
    int getX() const;
    int getY() const;
    void setX(const int nx);
    void setY(const int ny);
    Vec2i operator + (const Vec2i& right);
    Vec2i operator - (const Vec2i& right);
    Vec2i operator * (int val);
    Vec2i operator = (const Vec2i& right);
    double abs();
    void luaPush(lua_State *state);
private:
    int getX(lua_State *state);
    int getY(lua_State *state);
    int x, y;
};

}
