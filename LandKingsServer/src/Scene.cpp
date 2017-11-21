#include "Scene.h"
#include "GameObject.h"

using namespace Engine;

//public methods

template<class T>
using method = int (T::*)(lua_State *L);

template<class T, method<T> func>
int dispatch(lua_State *L) {
    T* ptr = *static_cast<T**>(lua_getextraspace(L));
    return ((*ptr).*func)(L);
}

typedef int (Scene::*mem_func)(lua_State * L);

template <mem_func func>
int dispatch(lua_State * L) {
    Scene * ptr = *static_cast<Scene**>(lua_getextraspace(L));
    return ((*ptr).*func)(L);
}

Scene::Scene() {
    height = Constants::SCENE_LENGTH;
    width = Constants::SCENE_HEIGHT;
}

void Scene::update() {
    for (auto object : objects) {
        object->update(*this);
    }
}

void Scene::addObject(PGameObject obj) {
    objects.push_back(obj);
}

void Scene::print() {
    for (auto object : objects) {
        std::cout << "char: " << object->getPosition().getX() << ' ' <<  object->getPosition().getY() << std::endl;
    }
}

void Scene::luaReg(lua_State *L) {
    Scene **Pscene = (Scene**)lua_newuserdata(L, sizeof(Scene*));
    *Pscene = this;
    if (luaL_newmetatable(L, "SceneMetaTable")) {
        lua_pushvalue(L, -1);
        lua_setfield(L, -2, "__index");

        luaL_Reg ScannerMethods[] = {
            "tist", &dispatch<Scene, test>,
            nullptr, nullptr
        };
        luaL_setfuncs(L, ScannerMethods, 0);
    }
    lua_setmetatable(L, -2);
    //lua_setglobal(L, "Scene");
}

void Scene::move(GameObject *object, Position &new_pos) {
    if (validPosition(new_pos)) {
        object->setPosition(new_pos);
    }
}

//private methods

bool Scene::validPosition(Position & pos) {
    return pos.getX() >= 0 &&
           pos.getY() >= 0 &&
           pos.getX() <= width &&
           pos.getY() <= height;
}

int Scene::test(lua_State *L) {
    Scene** Pscene = (Scene**)luaL_checkudata(L, 1, "SceneMetaTable");
    int x = luaL_checkinteger(L, 2);
    std::cout << x << " - passed by lua" << std::endl;
    return 0;
}
