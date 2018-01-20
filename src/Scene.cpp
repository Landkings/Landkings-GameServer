#include "Scene.h"
#include "GameObject.h"
#include "ObjectSpawner.h"
#include "SafeZone.h"

#include <iomanip>

#include "document.h"
#include "writer.h"
#include "stringbuffer.h"

using namespace Engine;
using namespace rapidjson;

//public methods

Scene::Scene() : grass(true, 1), land(true, 0), wall(false, 2),
    height(Constants::SCENE_HEIGHT  / Constants::TILE_HEIGHT),
    width(Constants::SCENE_WIDTH / Constants::TILE_WIDTH),
    time(0),
    characterSpawner(new ObjectSpawner(this, new Character(this, "", ""), Vec2i(0, 0), Vec2i(width * 4 /* * Constants::TILE_WIDTH*/,
                                                                                             height * 4 /* * Constants::TILE_HEIGHT*/))),
    safeZone(new SafeZone(this, Vec2i(0, 0))) {
    tiles.resize(height);
    spawners["heal"] = new ObjectSpawner(this, new HealingItem(this, Vec2i(), HitBox(5, 5), 10, 3, 1, 600, 5, 100),
                                         Vec2i(0, 0), Vec2i(width * Constants::TILE_WIDTH, height * Constants::TILE_HEIGHT), 100, 3000);

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
        if (!checkAllCollisions(object, &newPos))
            ((Character*)object)->move(newPos);
    }
}

void Scene::move(GameObject *object, GameObject *target) {
    Vec2i direction = findDirection(object, target);
    move(object, object->getPosition() + direction);
}

void Scene::attack(Character *c1, Character *c2) {
    if (!canAttack(c1, c2)) {
        Vec2i newPos = c1->getPosition() + findDirection(c1, c2);
        move(c1, newPos); //TODO: delete
    }
    else {
        if (c1->getNextAttackTime() <= time && c2->getHp() > 0) {
            c1->attack(c2);
        } //Shouldn't be else, since it checks hp after attack
        if (c2->getHp() <= 0) {
            c1->setTarget(nullptr);
            c1->gainExp(c2->getExpValue());
        }
    }
}

void Scene::update() {
    objectsMutex.lock();
    if (characters.size() == 0) {
        objectsMutex.unlock();
        restart();
        return;
    }
    safeZone->update();
    for (auto& spawner : spawners) {
        //spawner.second->spawn(objects);
    }

    for (auto& character : characters) {
        if (character->getNextStaminaRegenTime() <= time)
            character->gainDefaultStamina();

        if (!character->isOnCooldown())
            character->update();

        if (!(getTime() % 1000) && !safeZone->inZone(character)) {
            character->takeDamage(1);
        }
    }

    clearCorpses();
    objectsMutex.unlock();
    ++time;
}

void Scene::addObject(GameObject *obj) {
    objects.push_back(obj);
}

void Scene::addPlayer(std::string playerName, std::string luaCode) {
    Character* player;
    objectsMutex.lock();
    if (players.find(playerName) == players.end() || !(player = (Character*)getPlayer(playerName))) {
        player = (Character*)characterSpawner->spawn(characters);
        player->loadLuaCode(luaCode);
        player->setName(playerName);
        players.insert(std::make_pair(playerName, luaCode));
        //characters.push_back(player);
    }
    else {
        player->loadLuaCode(luaCode);
    }
    objectsMutex.unlock();
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
            "canAttack", dispatch<Scene, &Scene::luaCanAttack>,
            //"test", dispatch<Scene, &Scene::test>,
            nullptr, nullptr
        };
        luaL_setfuncs(L, SceneMethods, 0);
    }
    lua_setmetatable(L, -2);
    //lua_setglobal(L, "Scene");
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
            if (isCollide(*newPos, obj->getHitbox(),  object->getPosition(), object->getHitbox()))
                return true;

    for (auto& character : characters)
        if (character != obj)
            if (isCollide(*newPos, obj->getHitbox(),  character->getPosition(), character->getHitbox()))
                return true;

    return checkSceneCollision(obj, newPos);
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
            //players.erase(characters[i]->getID()); //comment to respawn
            delete characters[i];
            characters.erase(characters.begin() + i);
        }
    }
}

void Scene::restart() {
    objectsMutex.lock();
    for (auto& player : players) {
        Character *obj = (Character*)characterSpawner->spawn(characters);
        obj->loadLuaCode(player.second);
        obj->setName(player.first);
    }
    objectsMutex.unlock();
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

void Scene::createObjectsMessage(StringBuffer& buffer) {
    objectsMutex.lock();
    Document doc(kObjectType);
    doc.SetObject();
    Document::AllocatorType& allc = doc.GetAllocator();

    Value messageType(kStringType);
    messageType.SetString("loadObjects");
    doc.AddMember("messageType", messageType, allc);

    Value players(kArrayType);
    Value nick(kStringType);
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
        players.PushBack(player, allc);
    }
    doc.AddMember("players", players, allc);

    Writer<StringBuffer> writer(buffer);
    doc.Accept(writer);
    objectsMutex.unlock();
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

//lua methods

int Scene::luaGetObjects(lua_State *L) {
    Character* player = *static_cast<Character**>(lua_getextraspace(L));
    lua_newtable(L);
    int i = 1;
    for (auto& object : characters) {
        if (object != player && (player->getPosition() - object->getPosition()).abs() <= player->getVisionRange()) {
            ((Character*)object)->luaPush(L);
            lua_rawseti(L, -2, i++);
        }
        //lua_pushinteger(L, i++);
    }
    return 1;
}

int Scene::luaCanAttack(lua_State *state) {
    Character* player = *static_cast<Character**>(lua_getextraspace(state));
    Character* target = (Character*)lua_touserdata(state, -1);
    lua_pushboolean(state, canAttack(player, target));
    return 1;
}
