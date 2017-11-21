#pragma once
#include <string>
#include <memory>
#include <functional>

#include "Scene.h"
#include "Position.h"
#include "Hitbox.h"
#include "lua.hpp"

namespace Engine {

enum Action {
    Empty = -1,
    Move = 0,
};

enum Direction {
    Unknown = -1,
    Up = 0,
    Right = 1,
    Down = 2,
    Left =3
};

class GameObject {
public:
    GameObject(Scene* scene, Position pos = Position(), HitBox hbox = HitBox());
    Position getPosition() const { return position; }
    void setPosition(Position pos) { position = pos; }
    HitBox getHitbox() { return hbox; }
    void setHitbox(HitBox& hitbox) { hbox = hitbox; }
    std::string getName() { return name; }

    virtual bool isPassable() { return false; }
    virtual void update(Scene &scene) = 0;

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
    void move(Scene& scene);
    int test(lua_State *state);
	void update(Scene& scene) override;
    int getSpeed() { return speed; }
protected:
    int move(lua_State *state);
    Action action;
    int hitPoints;
    int speed;
    lua_State *L;
};
typedef std::shared_ptr<Character> PCharacter;

}
