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
    FILE * f = fopen("input.txt", "w");
    fprintf(f, "hello");
    if (luaL_loadfile(L, "test.lua")) {
        printf("Error loading script\n");
    }
    luaL_openlibs(L);
    lua_pushcfunction(L, &dispatch<&Character::move>);
    lua_setglobal(L, "test");
}

void Character::move() {
    lua_pcall(L, 0, 0, 0);
    printf("Helper\n");
}

// private methods

int Character::move(lua_State *state) {
    printf("Main move\n");
    scene->move(this);
    return 0;
}

int Character::test(lua_State *state) {
    printf("test\n");
    return 0;
}

//Obstacle::Obstacle() {}
