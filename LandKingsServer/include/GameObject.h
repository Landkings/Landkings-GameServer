#pragma once
#include <string>
#include <memory>
#include <functional>

#include "Scene.h"
#include "Position.h"
#include "Hitbox.h"
#include "lua.hpp"

namespace Engine {

class GameObject {
public:
    GameObject(Scene* scene, Position pos = Position(), HitBox hbox = HitBox());
    Position getPosition() { return position; }
    void setPosition(Position& pos) { position = pos; }
    HitBox getHitbox() { return hbox; }
    void setHitbox(HitBox& hitbox) { hbox = hitbox; }
    std::string getName() { return name; }
    virtual bool isPassable() { return false; }
protected:
    Position position;
    HitBox hbox;
    Scene *scene;
    std::string name; //?
};
//typedef std::shared_ptr<GameObject> PGameObject;

//class Obstacle : public GameObject {
//public:
//    Obstacle();
//protected:

//};

//class Surface : public GameObject {
//public:
//    Surface();
//protected:
//
//}

class Character : public GameObject {
public:
    Character(Scene *scene, Position pos = Position(), HitBox hbox = HitBox());
    void move();
    int test(lua_State *state);
protected:
    int move(lua_State *state);
    int hitPoints;
    int speed;
    lua_State *L;
};
typedef std::shared_ptr<Character> PCharacter;

}
