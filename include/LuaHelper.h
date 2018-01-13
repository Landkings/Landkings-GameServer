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

template<class T>
using hookMethod = void (T::*)(lua_State *L, lua_Debug *ar);

template<class T>
using continuationMethod = int (T::*)(lua_State *L, int status, lua_KContext ctx);

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

template<class T, hookMethod<T> func>
void dispatchHook(lua_State *L, lua_Debug *ar) {
    T* Pobj = *((T**)lua_touserdata(L, 1));
    ((*Pobj).*func)(L, ar);
}

template<class T, continuationMethod<T> func>
int dispatchContinuation(lua_State *L, int status, lua_KContext ctx) {
    T* Pobj = *((T**)lua_touserdata(L, 1));
    return ((*Pobj).*func)(L, status, ctx);
}

}
