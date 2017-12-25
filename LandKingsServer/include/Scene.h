#pragma once
#include <iostream>
#include <vector>
#include <memory>
#include <mutex>
#include <set>

#include "stringbuffer.h"

#include "Constants.h"
#include "lua.hpp"
#include "Position.h"
#include "Tile.h"
#include "LuaHelper.h"

namespace Engine {

class GameObject;
typedef std::shared_ptr<GameObject> PGameObject;
using TileMap = std::vector<std::vector<Tile*>>;
class Character;

class Scene {
public:
    Scene();
    void move(GameObject *object, const Position &new_pos);
    void move(GameObject *object, GameObject *target);
    void attack(Character *c1, Character *c2);
    void update();
    void addObject(GameObject *obj);
    void addPlayer(std::string playerName, std::string luaCode);
    void print();
    void luaReg(lua_State *L);
    const TileMap& getTileMap() const { return tiles; }
    const std::vector<GameObject*>& getObjects() const;
    long long getTime() const { return time; }
    void getObjectsJSON(rapidjson::StringBuffer& buffer);
    void getTileMapJSON(rapidjson::StringBuffer& buffer);
private:
    bool validPosition(const Position &pos, const HitBox &hbox);
    int getObjects(lua_State *L);
    Position findDirection(GameObject *from, GameObject *to);
    bool isCollide(const GameObject *first, const GameObject *second);
    bool isCollide(const Position firstPos, const HitBox firstHitBox, const Position secondPos, const HitBox secondHitBox);
    bool isCollide(const Position firstPos, const int firstWidth, const int firstHeight, const Position secondPos, const int secondWidth, const int secondHeight);
    bool checkSceneCollision(const GameObject *obj, const Position *newPos);
    GameObject *getPlayer(std::string& playerName);
    void clearCorpses();
    Position getRandomPosition();
    std::vector<GameObject*> objects;
    std::set<std::string> players;
    TileMap tiles;
    int height;
    int width;
    long long time;
    std::mutex objectsMutex;
    //todo replace
    Tile land;
    Tile wall;
    Tile grass;
};

}
