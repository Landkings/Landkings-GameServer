#include "GameObject.h"

using namespace Engine;

typedef int (Character::*mem_func)(lua_State * L);

template <mem_func func>
int dispatch(lua_State * L) {
    Character * ptr = *static_cast<Character**>(lua_getextraspace(L));
    return ((*ptr).*func)(L);
}

// public methods

GameObject::GameObject(Scene *scene, Position pos, HitBox hbox) : scene(scene), position(pos), hbox(hbox) {}

Character::Character(Scene *scene, Position pos, HitBox hbox) : GameObject(scene, pos, hbox) {
    L = luaL_newstate();
    *static_cast<Character**>(lua_getextraspace(L)) = this;
    luaL_openlibs(L);
    lua_pushcfunction(L, dispatch<&Character::test>);
    lua_setglobal(L, "test");
    scene->luaReg(L);
    if (luaL_loadfile(L, "test.lua")) {
        printf("Error loading script\n");
    }
}

void Character::update(Scene &scene) {
    move();
}

// private methods

void Character::move(Scene &scene) {
    Position direction[4] = {
        Position(1, 0),
        Position(0, 1),
        Position(-1,0),
        Position(0, -1)
    };
    scene.move(this, direction[0]);
}

void Character::move() {
    lua_getglobal(L, "move");
    lua_pushlightuserdata(L, (void *)&scene);
    lua_pcall(L, 0, 0, 0);
    //printf("Helper\n");
}

// private methods

int Character::move(lua_State *state) {
    printf("Main move\n");
    int x = lua_tonumber(L, 1);
    int y = lua_tonumber(L, 2);
    //int* y = lua_tointeger(L, 2);
    //Position direction[4] = {
    //    Position(1, 0),
    //    Position(0, 1),
    //    Position(-1,0),
    //    Position(0, -1)
    //};
    Position p = position + Position(x, y);
    scene->move(this, p); // TODO: figure out how to directly pass position + Position(x, y) as argument
    return 0;
}

int Character::test(lua_State *state) {
    printf("test\n");
    return 0;
}

//Obstacle::Obstacle() {}
