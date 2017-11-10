#pragma once
#include <string>
#include <memory>

#include "Position.h"
#include "Hitbox.h"

namespace Engine {

class GameObject {
public:
    GameObject(Position pos = Position(), HitBox hbox = HitBox());
    Position getPosition() { return position; }
    void setPosition(Position& pos) { position = pos; }
    HitBox getHitbox() { return hbox; }
    void setHitbox(HitBox& hitbox) { hbox = hitbox; }
    std::string getName() { return name; }
    virtual bool isPassable() { return false; }
protected:
    Position position;
    HitBox hbox;
    std::string name; //?
};
typedef std::shared_ptr<GameObject> PGameObject;

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
    Character(Position pos = Position(), HitBox hbox = HitBox());
protected:
    int hitPoints;
    int speed;
};
typedef std::shared_ptr<Character> PCharacter;

}
