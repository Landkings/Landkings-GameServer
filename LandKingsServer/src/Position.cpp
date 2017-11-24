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

void Position::luaPush(lua_State *state) {
    Position **Pposition = (Position**)lua_newuserdata(state, sizeof(Position*));
    *Pposition = this;
    if (luaL_newmetatable(state, "PositionMetaTable")) {
        lua_pushvalue(state, -1);
        lua_setfield(state, -2, "__index");

        luaL_Reg PositionMethods[] = {
            "getX", &dispatch<Position, getX>,
            "getY", &dispatch<Position, getY>,
            nullptr, nullptr
        };
        luaL_setfuncs(state, PositionMethods, 0);
    }
    lua_setmetatable(state, -2);
}

int Position::getX(lua_State *state) {
    Position** Pposition = (Position**)luaL_checkudata(state, 1, "PositionMetaTable");
    lua_pushinteger(state, Pposition[0]->getX());
    return 1;
}

int Position::getY(lua_State *state) {
    Position** Pposition = (Position**)luaL_checkudata(state, 1, "PositionMetaTable");
    lua_pushinteger(state,  Pposition[0]->getY());
    return 1;
}
