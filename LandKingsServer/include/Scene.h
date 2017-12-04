#pragma once
#include <iostream>
#include <vector>
#include <memory>

#include "Constants.h"
#include "lua.hpp"
#include "Position.h"
#include "Tile.h"
#include "LuaHelper.h"

namespace Engine {

class GameObject;
typedef std::shared_ptr<GameObject> PGameObject;
class Character;

class Scene {
public:
    Scene();
    void move(GameObject *object, const Position &new_pos);
    void move(GameObject *object, GameObject *target);
    void attack(Character *c1, Character *c2);
    void update();
    //void addObject(PGameObject obj);
    void addObject(GameObject *obj);
    void print();
    void luaReg(lua_State *L);
    const std::vector<GameObject*>& getObjects() const;
    long long getTime() const { return time; }
private:
    bool validPosition(const Position &pos);
    //int test(lua_State *L);
    int getObjects(lua_State *L);
    Position findDirection(GameObject *from, GameObject *to);
    bool isCollide(const GameObject *first, const GameObject *second);
    bool isCollide(const Position firstPos, const HitBox firstHitBox, const Position secondPos, const HitBox secondHitBox);
    bool isCollide(const Position firstPos, const int firstWidth, const int firstHeight, const Position secondPos, const int secondWidth, const int secondHeight);
    bool checkSceneCollision(const GameObject *obj, const Position *newPos);
    //std::vector<PGameObject> objects;
    std::vector<GameObject*> objects;
    std::vector<std::vector<Tile*>> tiles;
    int height;
    int width;
    long long time;
    //todo replace
    Tile land;
    Tile wall;
};

}
