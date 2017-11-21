#pragma once
#include <iostream>
#include <vector>
#include <memory>

#include "Constants.h"
#include "lua.hpp"
#include "Position.h"

namespace Engine {

class GameObject;
typedef std::shared_ptr<GameObject> PGameObject;

class Scene {
public:
    Scene();
    void move(GameObject *object, Position &new_pos);
    void update();
    void addObject(PGameObject obj);
    void print();
    void luaReg(lua_State *L);
private:
    bool validPosition(Position &pos);
    int test(lua_State *L);
    std::vector<PGameObject> objects;
    int height;
    int width;
};

}
