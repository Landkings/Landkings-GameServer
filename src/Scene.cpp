#include "Scene.h"
#include "GameObject.h"
#include "ObjectSpawner.h"
#include "SafeZone.h"

#include <iomanip>
#include <thread>
#include <string>
#include <sstream>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

using namespace Engine;
using namespace rapidjson;

//public methods

Scene::Scene() : grass(true, 1), land(true, 0), wall(false, 2),
    height(Constants::SCENE_HEIGHT / Constants::TILE_HEIGHT),
    width(Constants::SCENE_WIDTH / Constants::TILE_WIDTH),
    time(0),
//    characterSpawner(new ObjectSpawner(this, new Character(this, "", ""), Vec2i(0, 0), Vec2i(width * 4 /* * Constants::TILE_WIDTH*/,
//                                                                                          height * 4 /* * Constants::TILE_HEIGHT*/))),
    safeZone(new SafeZone(this, Vec2i(std::rand() % (Constants::SCENE_HEIGHT / 10) , rand() % (Constants::SCENE_WIDTH / 10)))) {
    std::srand(unsigned(std::time(0)));
    tiles.resize(height);
    //spawners["heal"] = new ObjectSpawner(this, new HealingItem(this, Vec2i(), HitBox(5, 5), 10, 3, 1, 600, 5, 100, 98),
    //                                     Vec2i(0, 0), Vec2i(width * Constants::TILE_WIDTH, height * Constants::TILE_HEIGHT), 100, 3000);
    spawners["exp"] = new ObjectSpawner(this, new ExpItem(this, Vec2i(), HitBox(5, 5), 100, 1, 1, 0, 0, 0, 99),
                                        Vec2i(10, 10), Vec2i(width * Constants::TILE_WIDTH - 10, height * Constants::TILE_HEIGHT - 10), 100, 30000);
    characterSpawners["player"] = new ObjectSpawner(this, new Player(this, "", ""), Vec2i(0, 0), Vec2i(width * 4 /* * Constants::TILE_WIDTH*/,
                                                                                             height * 4 /* * Constants::TILE_HEIGHT*/));
    characterSpawners["character"] = new ObjectSpawner(this, new Character(this, "", ""), Vec2i(0, 0), Vec2i(width * 4 /* * Constants::TILE_WIDTH*/,
                                                                                                       height * 4 /* * Constants::TILE_HEIGHT*/));

    //TOOD: add loading map from somewhere or generating
    //int k = 0; //delete
    for (auto& row : tiles) {
        //row.resize(width, (k++ % 2) ? &land : &grass);
        row.resize(width, &grass);
    }
    tiles[1][4] =  &land;
//    tiles[10][13] =  &wall;
}

void Scene::move(GameObject *object, const Vec2i &newPos) {
    if (((Character*)object)->getNextMoveTime() <= time && validPosition(newPos, object->getHitbox())) {
        //((Character*)object)->setNextMoveTime();
        //if (!checkAllCollisions(object, &newPos))
        bool isPassable = true;
        Item* item = nullptr;
        for (auto& obj : objects)
            if (object != obj) {
                if (isCollide(newPos, object->getHitbox(),  obj->getPosition(), obj->getHitbox())) {
                    if (obj->isPassable()) {
                        item = (Item*)obj;
                        break;
                    }
                    else {
                        isPassable = false;
                        break;
                    }
                }
            }
        if (isPassable)
            for (auto& character : characters)
                if (character != object)
                    if (isCollide(newPos, object->getHitbox(),  character->getPosition(), character->getHitbox())) {
                        isPassable = false;
                        break;
                    }

        if (isPassable)
            isPassable = !checkSceneCollision(object, &newPos);

        if (isPassable) {
            if (item) {
                ((Character*)object)->takeItem(item);
            }
            ((Character*)object)->move(newPos);
        }
    }
}

void Scene::move(GameObject *object, GameObject *target) {
    Vec2i direction = findDirection(object, target);
    move(object, object->getPosition() + direction);
}

void Scene::attack(Character *c1, Character *c2) {
        if (!c1->isUsingAction() && !canAttack(c1, c2)) {
            Vec2i newPos = c1->getPosition() + findDirection(c1, c2);
            move(c1, newPos); //TODO: delete
        }
        else {
            if (c1->getNextAttackTime() <= time && c2->getHp() > 0) {
                if (c1->isUsingAction())
                    c1->payAttackCost();
                c1->attack(c2);
            } //Shouldn't be else, since it checks hp after attack
            if (c2->getHp() <= 0) {
                c1->setTarget(nullptr);
                c1->gainExp(c2->getExpValue());
            }
        }
}

void Scene::update() {
    acquireObjects();
    if (characters.size() <= 1 && players.size() > 1) {
        restart();
        releaseObjects();
        return;
    }
    safeZone->update();

//    std::ifstream input("p1.lua", std::ios::binary);
//    std::stringstream luaCode;
//    luaCode << input.rdbuf();
//    Character *character;
//    character = (Character*)characterSpawner->spawn(characters);
//    character->loadLuaCode(luaCode.str());
//    character->setName("monster");
//
//    for (auto& spawner : spawners) {
//        spawner.second->spawn(objects);
//    }

    for (auto& character : characters) {
        if (character->getNextStaminaRegenTime() <= time)
            character->gainDefaultStamina();
        character->enableStaminaRegen();

        if (!character->isOnCooldown())
            character->update();

        if (!safeZone->inZone(character)) {
            if (!(getTime() % 500))
                character->takeDamage(1);
            character->disableStaminaRegen();
        }
    }
    clearCorpses();
    ++time;
    releaseObjects();
}

void Scene::addObject(GameObject *obj) {
    objects.push_back(obj);
}

void Scene::spawnPlayer(std::string name, std::string luaCode) {
    Player* player = (Player*)characterSpawners["player"]->spawn(characters);
    player->loadLuaCode(luaCode);
    player->setName(name);
    players.insert(std::make_pair(name, luaCode));
}

void Scene::spawnCharacter(std::string name, std::string luaCode) {
    Character *character = (Character*)characterSpawners["character"]->spawn(characters);
    character->setName(name);
    character->loadLuaCode(luaCode);
}

void Scene::addPlayer(std::string playerName, std::string luaCode) {
    Character* player;
    acquireObjects();
    auto it = players.find(playerName);
    if (it == players.end() || !(player = (Character*)getPlayer(playerName))) {
        spawnPlayer(playerName, luaCode);
//        characters.push_back(player);
    }
    else {
        it->second = luaCode;
        player->loadLuaCode(luaCode);
    }
    releaseObjects();
}

void Scene::luaPush(lua_State *L) {
    Scene **Pscene = (Scene**)lua_newuserdata(L, sizeof(Scene*));
    //void* p = this;
    //lua_pushlightuserdata(L, this);
    *Pscene = this;
    if (luaL_newmetatable(L, "SceneMetaTable")) {
        lua_pushvalue(L, -1);
        lua_setfield(L, -2, "__index");

        luaL_Reg SceneMethods[] = {
            "getObjects", dispatch<Scene, &Scene::luaGetObjects>,
            //"canAttack", dispatch<Scene, &Scene::luaCanAttack>,
            "getSafeZone", dispatch<Scene, &Scene::luaGetSafeZone>,
            "getWidth", dispatch<Scene, &Scene::luaGetWidth>,
            "getHeight", dispatch<Scene, &Scene::luaGetHeight>,
            //"test", dispatch<Scene, &Scene::test>,
            nullptr, nullptr
        };
        luaL_setfuncs(L, SceneMethods, 0);
    }
    lua_setmetatable(L, -2);
    //lua_setglobal(L, "Scene");
}

void Scene::takeItem(Character *c, Item *i) {
    //if (canTakeItem(c, i))
    //    c->takeItem(i);
}

//private methods

bool Scene::validPosition(const Vec2i &pos, const HitBox &hbox) {
    return pos.getX() - (hbox.getWidth() / 2)  >= 0 &&
           pos.getY() - (hbox.getHeight() / 2) >= 0 &&
           pos.getX() + (hbox.getWidth() / 2)  < Constants::SCENE_WIDTH &&
           pos.getY() + (hbox.getHeight() / 2) < Constants::SCENE_HEIGHT;
}

Vec2i Scene::findDirection(GameObject *from, GameObject *to) {
    int diff;
    if ((diff = to->getX() - from->getX())) {
        return Vec2i(diff/abs(diff), 0);
    }
    else if ((diff = to->getY() - from->getY())) {
        return Vec2i(0, diff/abs(diff));
    }
}

bool Scene::isCollide(const GameObject *first, const GameObject *second) {
    return isCollide(first->getPosition(), first->getHitbox(), second->getPosition(), second->getHitbox());
}

bool Scene::isCollide(const Vec2i firstPos, const HitBox firstHitBox, const Vec2i secondPos, const HitBox secondHitBox) {
    return isCollide(firstPos, firstHitBox.getWidth(), firstHitBox.getHeight(), secondPos, secondHitBox.getWidth(), secondHitBox.getHeight());
}

bool Scene::isCollide(const Vec2i firstPos, const int firstWidth, const int firstHeight, const Vec2i secondPos, const int secondWidth, const int secondHeight) {
    return ((firstPos.getX() - secondPos.getX()) < (secondWidth + firstWidth) / 2) &&
           ((secondPos.getX() - firstPos.getX()) < (firstWidth + secondWidth) / 2) &&
           ((firstPos.getY() - secondPos.getY()) < (secondHeight + firstHeight) / 2) &&
           ((secondPos.getY() - firstPos.getY()) < (firstHeight + secondHeight) / 2);
}

bool Scene::checkSceneCollision(const GameObject *obj, const Vec2i *newPos) {
    for (int i = (newPos->getY() - obj->getHeight() / 2) / Constants::TILE_HEIGHT;
         i <= (newPos->getY() + obj->getHeight() / 2) / Constants::TILE_HEIGHT; ++i)
        for (int j = (newPos->getX() - obj->getWidth() / 2) / Constants::TILE_WIDTH;
             j <= (newPos->getX() + obj->getWidth() / 2) / Constants::TILE_WIDTH; ++j)
            if (!tiles[i][j]->isPassable())
                return true;

    return false;
}

bool Scene::checkAllCollisions(const GameObject *obj, const Vec2i *newPos) {
    for (auto& object : objects)
        if (object != obj)
            if (!object->isPassable() && isCollide(*newPos, obj->getHitbox(),  object->getPosition(), object->getHitbox()))
                return true;

    for (auto& character : characters)
        if (character != obj)
            if (isCollide(*newPos, obj->getHitbox(),  character->getPosition(), character->getHitbox()))
                return true;

    return checkSceneCollision(obj, newPos);
}

bool Scene::canMove(Character *c, Vec2i newPos) {
    return c->getNextMoveTime() <= time && validPosition(newPos, c->getHitbox()) &&
        !checkAllCollisions(c, &newPos);
}

GameObject *Scene::getPlayer(std::string &playerName) {
    for (auto& obj: characters)
        if (obj->getName() == playerName)
            return obj;

    return nullptr;
}

void Scene::clearCorpses() {
    for (int i = characters.size() - 1; i >= 0; --i) {
        if (characters[i]->getHp() <= 0) {
//            players.erase(characters[i]->getID()); //comment to respawn
            delete characters[i];
            characters.erase(characters.begin() + i);
        }
    }
}

void Scene::restart() {
    delete safeZone;
    for (int i = 0; i < characters.size(); ++i)
        delete characters[i];
    characters.clear();

    for (int i = 0; i < objects.size(); ++i) {
        delete objects[i];
    }
    objects.clear();

    for (auto& spawner : spawners) {
        //spawner.second->spawn(objects);
        spawner.second->clear();
    }

    characters.clear();
    objects.clear();
    srand(std::time(0));
    safeZone = new SafeZone(this, Vec2i(rand() % (Constants::SCENE_HEIGHT / 10), rand() % (Constants::SCENE_WIDTH / 10)));
    srand(std::time(0));
    for (auto& player : players) {
        spawnCharacter(player.first, player.second);
    }
}

bool Scene::canAttack(Character *c1, Character *c2) {
    for (int i = 0; i < 4; ++i) {
        Vec2i direction = directions[i];
        HitBox attackHitBox(abs(direction.getX() * c1->getAttackRange()), abs(direction.getY() * c1->getAttackRange()));
        Vec2i attackPosition = c1->getPosition() + direction * (c1->getAttackRange() / 2);
        if (isCollide(attackPosition, attackHitBox, c2->getPosition(), c2->getHitbox()))
            return true;
    }
    return false;
}

Vec2i Scene::getRandomEmptyPosition() {
    //TODO: implement
    return Vec2i();
}

void Scene::createObjectsMessage(StringBuffer& buffer) {
    {
        bool cur;
        while (!objectsAcquired.compare_exchange_strong(cur, true))
        {
            cur = false;
            std::this_thread::sleep_for(std::chrono::nanoseconds(5));
        }
    }
    Document doc(kObjectType);
    doc.SetObject();
    Document::AllocatorType& allc = doc.GetAllocator();

    Value messageType(kStringType);
    messageType.SetString("loadObjects");
    doc.AddMember("messageType", messageType, allc);

    Value players(kArrayType);
    Value npcs(kArrayType);
    Value nick(kStringType);
    Value circle(kObjectType);
    Value items(kArrayType);
    for (auto& character : characters) {
        Value player(kObjectType);
        player.AddMember("x", character->getX(), allc);
        player.AddMember("y", character->getY(), allc);
        player.AddMember("hp", character->getHp(), allc);
        player.AddMember("st", character->getStamina(), allc);
        player.AddMember("mhp", character->getMaxHp(), allc);
        player.AddMember("mst", character->getMaxStamina(), allc);
        player.AddMember("sid", (int)character->getSpriteDirection(), allc);
        player.AddMember("lv", character->getLevel(), allc);
        nick.SetString(character->getID().data(), allc);
        player.AddMember("id", nick, allc);
        if (character->getType() == ObjectType::Player)
            players.PushBack(player, allc);
        else
            npcs.PushBack(player, allc);
    }
    doc.AddMember("players", players, allc);
    doc.AddMember("npcs", npcs, allc);

    for (auto& object : objects) {
        Value item(kObjectType);
        item.AddMember("x", object->getX(), allc);
        item.AddMember("y", object->getY(), allc);
        item.AddMember("sid", ((Item*)object)->getSpriteId(), allc);
        items.PushBack(item, allc);
    }
    doc.AddMember("items", items, allc);

    circle.AddMember("x", safeZone->getPosition().getX(), allc);
    circle.AddMember("y", safeZone->getPosition().getY(), allc);
    circle.AddMember("r", safeZone->getRadius(), allc);
    doc.AddMember("circle", circle, allc);

    Writer<StringBuffer> writer(buffer);
    doc.Accept(writer);
    releaseObjects();
}

void Scene::createMapMessage(StringBuffer& buffer) {
    Document doc;
    doc.SetObject();
    Document::AllocatorType& allc = doc.GetAllocator();

    Value messageType(kStringType);
    messageType.SetString("loadMap", allc);
    doc.AddMember("messageType", messageType, allc);

    int height = tiles.size(), width = tiles[0].size();
    Value widthVal(kNumberType), heightVal(kNumberType);
    heightVal.SetInt(height); widthVal.SetInt(width);
    doc.AddMember("height", heightVal, allc);
    doc.AddMember("width", widthVal, allc);

    Value tileMap(kArrayType);
    Value tileElem(kNumberType);
    for (int i = 0; i < height; ++i)
    {
        for (int j = 0; j < width; ++j)
        {
            tileElem.SetInt(tiles[i][j]->getIdx());
            tileMap.PushBack(tileElem, allc);
        }
    }
    doc.AddMember("tileMap", tileMap, allc);

    Writer<StringBuffer> writer(buffer);
    doc.Accept(writer);
}

void Scene::acquireObjects()
{
    bool cur = false;
    while (!objectsAcquired.compare_exchange_strong(cur, true))
    {
        cur = false;
        std::this_thread::sleep_for(std::chrono::nanoseconds(25));
    }
}

void Scene::releaseObjects()
{
    objectsAcquired.exchange(false);
}

//lua methods

int Scene::luaGetObjects(lua_State *L) {
    Character* player = *static_cast<Character**>(lua_getextraspace(L));
    lua_newtable(L);
    int i = 1;
    for (auto& object : characters) {
//        if (object != player && (player->getPosition() - object->getPosition()).abs() <= player->getVisionRange()) {
            ((Character*)object)->luaPush(L);
            lua_rawseti(L, -2, i++);
//        }
        //lua_pushinteger(L, i++);
    }
    return 1;
}

int Scene::luaGetSafeZone(lua_State *state) {
    safeZone->luaPush(state);
    return 1;
}

int Scene::luaGetWidth(lua_State *state) {
    lua_pushinteger(state, width * Constants::TILE_WIDTH);
    return 1;
}

int Scene::luaGetHeight(lua_State *state) {
    lua_pushinteger(state, height * Constants::TILE_HEIGHT);
    return 1;
}
