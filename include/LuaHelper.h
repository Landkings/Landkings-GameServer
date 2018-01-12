#pragma once

#include "lua.hpp"

namespace Engine {

#define LUA_ENUM(L, name, val) \
  lua_pushlstring(L, #name, sizeof(#name)-1); \
  lua_pushnumber(L, val); \
  lua_settable(L, -3);

#define C_ENUM_HELPER(cname, luaname)  cname,
#define LUA_ENUM_HELPER(cname, luaname) LUA_ENUM(L, luaname, cname)
#define C_ENUM_TO_STRING_HELPER(cname, luaname) { cname, #luaname },

template<class T>
using method = int (T::*)(lua_State *L);

template<class T, method<T> func>
int gloablDispatch(lua_State *L) {
    T* ptr = *static_cast<T**>(lua_getextraspace(L));
    return ((*ptr).*func)(L);
}

template<class T, method<T> func>
int dispatch(lua_State *L) {
    T* Pobj = *((T**)lua_touserdata(L, 1));
    return ((*Pobj).*func)(L);
}

}
