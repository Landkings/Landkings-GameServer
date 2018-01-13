#pragma once
#include <iostream>
#include <vector>
#include <memory>
#include <mutex>
#include <set>
#include <unordered_map>

#include "stringbuffer.h"

#include "Constants.h"
#include "lua.hpp"
#include "Vec2i.h"
#include "Tile.h"
#include "LuaHelper.h"

namespace Engine {

class GameObject;
typedef std::shared_ptr<GameObject> PGameObject;
using TileMap = std::vector<std::vector<Tile*>>;
class Character;
class Item;
class ObjectSpawner;

class Scene {
public:
    Scene();
    void move(GameObject *object, const Vec2i &newPos);
    void move(GameObject *object, GameObject *target);
    void attack(Character *c1, Character *c2);
    void update();
    void addObject(GameObject *obj);
    void addPlayer(std::string playerName, std::string luaCode);
    void luaPush(lua_State *L);
    long long getTime() const { return time; }
    void createObjectsMessage(rapidjson::StringBuffer& buffer);
    void createMapMessage(rapidjson::StringBuffer& buffer);
    bool checkAllCollisions(const GameObject *obj, const Vec2i *newPos);
private:
    bool validPosition(const Vec2i &pos, const HitBox &hbox);
    Vec2i findDirection(GameObject *from, GameObject *to);
    bool isCollide(const GameObject *first, const GameObject *second);
    bool isCollide(const Vec2i firstPos, const HitBox firstHitBox, const Vec2i secondPos, const HitBox secondHitBox);
    bool isCollide(const Vec2i firstPos, const int firstWidth, const int firstHeight, const Vec2i secondPos, const int secondWidth, const int secondHeight);
    bool checkSceneCollision(const GameObject *obj, const Vec2i *newPos);
    GameObject *getPlayer(std::string& playerName);
    void clearCorpses();
    bool canAttack(Character *c1, Character *c2);

    //lua methods
    int luaGetObjects(lua_State *L);
    int luaCanAttack(lua_State *L);

    //Vec2i getRandomPosition();
    std::vector<GameObject*> objects;
    std::vector<Character*> characters;
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
    std::unordered_map<std::string, ObjectSpawner*> spawners;
    ObjectSpawner *characterSpawner;
};

}
