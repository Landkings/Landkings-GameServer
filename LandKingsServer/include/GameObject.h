#pragma once
#include <string>
#include <memory>
#include "Point.h"

class GameObject {
public:
    GameObject();
    virtual Point getPosition();
    virtual std::string getName();
protected:
    Point position;
    std::string name; //?
};
typedef std::shared_ptr<GameObject> GameObjectPtr;

class Obstacle : public GameObject {
public:
    Obstacle();
protected:
    int length;
    int width;
};

class Character : public GameObject {
public:
    Character();
protected:
    int hitPoints;
    int speed;
};
