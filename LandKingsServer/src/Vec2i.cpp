#include "Vec2i.h"

using namespace Engine;

Vec2i::Vec2i(int x, int y) : x(x), y(y) {}

int Vec2i::getX() const { return x; }

int Vec2i::getY() const { return y; }

void Vec2i::setX(const int nx) { x = nx; }

void Vec2i::setY(const int ny) { y = ny; }

Vec2i Vec2i::operator+(const Vec2i& right) {
    return Vec2i(x + right.getX(), y + right.getY());
}

Vec2i Vec2i::operator-(const Vec2i& right) {
    return Vec2i(x - right.getX(), y - right.getY());
}

Vec2i Engine::Vec2i::operator*(int val) {
    return Vec2i(x * val, y * val);
}

Vec2i Vec2i::operator =(const Vec2i &right) {
    x = right.x;
    y = right.y;
    return *this;
}

double Vec2i::abs() {
    return std::sqrt(x*x + y*y);
}

void Vec2i::luaPush(lua_State *state) {
    Vec2i **Pposition = (Vec2i**)lua_newuserdata(state, sizeof(Vec2i*));
    *Pposition = this;
    if (luaL_newmetatable(state, "PositionMetaTable")) {
        lua_pushvalue(state, -1);
        lua_setfield(state, -2, "__index");

        luaL_Reg PositionMethods[] = {
            "getX", &dispatch<Vec2i, &Vec2i::getX>,
            "getY", &dispatch<Vec2i, &Vec2i::getY>,
            nullptr, nullptr
        };
        luaL_setfuncs(state, PositionMethods, 0);
    }
    lua_setmetatable(state, -2);
}

int Vec2i::getX(lua_State *state) {
    Vec2i** Pposition = (Vec2i**)luaL_checkudata(state, 1, "PositionMetaTable");
    lua_pushinteger(state, Pposition[0]->getX());
    return 1;
}

int Vec2i::getY(lua_State *state) {
    Vec2i** Pposition = (Vec2i**)luaL_checkudata(state, 1, "PositionMetaTable");
    lua_pushinteger(state,  Pposition[0]->getY());
    return 1;
}
