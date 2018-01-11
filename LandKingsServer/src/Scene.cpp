#include "Scene.h"
#include "GameObject.h"
#include <iomanip>

#include "document.h"
#include "writer.h"
#include "stringbuffer.h"

using namespace Engine;
using namespace rapidjson;

//public methods

Scene::Scene() : grass(true, 1), land(true, 0), wall(false, 2) {
    height = Constants::SCENE_HEIGHT  / Constants::TILE_HEIGHT;
    width = Constants::SCENE_WIDTH / Constants::TILE_WIDTH;
    time = 0;
    testIdx = 0;
    tiles.resize(height);
    int k = 0; //delete
    for (auto& row : tiles) {
        //row.resize(width, (k++ % 2) ? &land : &grass);
        row.resize(width, &grass);
    }
    tiles[1][4] =  &land;
//    tiles[10][13] =  &wall;
}

void Scene::move(GameObject *object, const Position &new_pos) {
    if (((Character*)object)->getNextMoveTime() <= time && validPosition(new_pos, object->getHitbox())) {
        //((Character*)object)->setNextMoveTime();
        if (!checkSceneCollision(object, &new_pos))
            ((Character*)object)->move(new_pos);
    }
}

void Scene::move(GameObject *object, GameObject *target) {
    Position direction = findDirection(object, target);
    move(object, object->getPosition() + direction);
}

void Scene::attack(Character *c1, Character *c2) {
    Position direction = findDirection(c1, c2);
    HitBox attackHitBox(abs(direction.getX() * c1->getAttackRange()), abs(direction.getY() * c1->getAttackRange()));
    Position newPos = c1->getPosition() + direction;
    Position attackPosition = c1->getPosition() + direction * (c1->getAttackRange() / 2);
    if (!isCollide(attackPosition, attackHitBox, c2->getPosition(), c2->getHitbox())) {
        move(c1, newPos);
    }
    else {
        if (c1->getNextAttackTime() <= time && c2->getHp() > 0) {
            c1->attack(c2);
        } //Shouldn't be else, since it checks hp after attack
        if (c2->getHp() <= 0) {
            c1->setTarget(nullptr);
        }
    }
}

void Scene::update() {
    objectsMutex.lock();
    for (auto& object : objects) {
        if (((Character*)object)->getNextStaminaRegenTime() <= time)
            ((Character*)object)->gainDefaultStamina();

        if (!((Character*)object)->isOnCooldown())
            ((Character*)object)->update();
    }
    clearCorpses();
    objectsMutex.unlock();
    ++time;
}

void Scene::addObject(GameObject *obj) {
    objects.push_back(obj);
}

void Scene::addPlayer(std::string playerName, std::string luaCode) {
    GameObject *player;
    objectsMutex.lock();
    if (players.find(playerName) == players.end()) {
        player = new Character(this, luaCode, playerName, getRandomPosition());
        players.insert(playerName);
        objects.push_back(player);
    }
    else {
        player = getPlayer(playerName);
        ((Character*)player)->loadLuaCode(luaCode);
    }
    objectsMutex.unlock();
}

void Scene::print() {
//    for (auto& object : objects)
//        std::cout << object->tmpLuaName << "\nHp: " << std::setw(3) << std::right << ((Character*)object)->getHp() << std::endl;

    std::vector<std::string> sc;
    for (int i = 0; i < 20; ++i) {
        std::string tmp = "";
        for (int j = 0; j < 25; ++j) {
            tmp += '.';
        }
        sc.push_back(tmp);
    }
    for (auto object : objects) {
        Position pos = object->getPosition();
        sc[pos.getY()][pos.getX()] = 'X';
    }
    for (auto& row : sc) {
        for (auto& col : row) {
            std::cout << col;
        }
        std::cout << "\n";
    }
    std::cout << std::flush;
}

void Scene::luaReg(lua_State *L) {
    Scene **Pscene = (Scene**)lua_newuserdata(L, sizeof(Scene*));
    //void* p = this;
    //lua_pushlightuserdata(L, this);
    *Pscene = this;
    if (luaL_newmetatable(L, "SceneMetaTable")) {
        lua_pushvalue(L, -1);
        lua_setfield(L, -2, "__index");

        luaL_Reg ScannerMethods[] = {
            "getObjects", dispatch<Scene, &Scene::getObjects>,
            "test", dispatch<Scene, &Scene::test>,
            nullptr, nullptr
        };
        luaL_setfuncs(L, ScannerMethods, 0);
        std::cout << "MetaTableCreated" << std::endl;
    }
    lua_setmetatable(L, -2);
    //lua_setglobal(L, "Scene");
}

//private methods

bool Scene::validPosition(const Position &pos, const HitBox &hbox) {
    return pos.getX() - (hbox.getWidth() / 2)  >= 0 &&
           pos.getY() - (hbox.getHeight() / 2) >= 0 &&
           pos.getX() + (hbox.getWidth() / 2)  < Constants::SCENE_WIDTH &&
           pos.getY() + (hbox.getHeight() / 2) < Constants::SCENE_HEIGHT;
}

int Scene::getObjects(lua_State *L) {
    Character* player = *static_cast<Character**>(lua_getextraspace(L));
    //Scene* Pscene = (Scene*)luaL_checkudata(L, 1, "SceneMetaTable");
    lua_newtable(L);
    //std::cout << "Table created" << std::endl;
    int i = 1;
    for (auto& object : objects) {
        if (object != player && (player->getPosition() - object->getPosition()).abs() <= player->getVisionRange()) {
            ((Character*)object)->luaPush(L);
            lua_rawseti(L, -2, i++);
        }
        //std::cout << i << " added to table" << std::endl;
        //lua_pushinteger(L, i++);
    }
    return 1;
}

Position Scene::findDirection(GameObject *from, GameObject *to) {
    int diff;
    if ((diff = to->getX() - from->getX())) {
        return Position(diff/abs(diff), 0);
    }
    else if ((diff = to->getY() - from->getY())) {
        return Position(0, diff/abs(diff));
    }
}

bool Scene::isCollide(const GameObject *first, const GameObject *second) {
    return isCollide(first->getPosition(), first->getHitbox(), second->getPosition(), second->getHitbox());
}

bool Scene::isCollide(const Position firstPos, const HitBox firstHitBox, const Position secondPos, const HitBox secondHitBox) {
    return isCollide(firstPos, firstHitBox.getWidth(), firstHitBox.getHeight(), secondPos, secondHitBox.getWidth(), secondHitBox.getHeight());
}

bool Scene::isCollide(const Position firstPos, const int firstWidth, const int firstHeight, const Position secondPos, const int secondWidth, const int secondHeight) {
    return firstPos.getX() < secondPos.getX() + secondWidth &&
           firstPos.getX() + firstWidth > secondPos.getX() &&
           firstPos.getY() < secondPos.getY() + secondHeight &&
           firstPos.getY() + firstHeight > secondPos.getY();
}

bool Scene::checkSceneCollision(const GameObject *obj, const Position *newPos) { //TODO AABB tree optimization
    for (int i = (newPos->getY() - obj->getHeight() / 2) / Constants::TILE_HEIGHT;
         i <= (newPos->getY() + obj->getHeight() / 2) / Constants::TILE_HEIGHT; ++i)
        for (int j = (newPos->getX() - obj->getWidth() / 2) / Constants::TILE_WIDTH;
             j <= (newPos->getX() + obj->getWidth() / 2) / Constants::TILE_WIDTH; ++j)
            if (!tiles[i][j]->isPassable())
                return true;

    return false;
}

GameObject *Scene::getPlayer(std::string &playerName) {
    for (auto& obj: objects) {
        if (obj->getName() == playerName) {
            return obj;
        }
    }
    return nullptr;
}

void Scene::clearCorpses() {
    for (int i = objects.size() - 1; i >= 0; --i) {
        if (((Character*)(objects[i]))->getHp() <= 0) {
            //players.erase(((Character*)(objects[i]))->getID()); //comment to respawn
            delete ((Character*)(objects[i]));
            objects.erase(objects.begin() + i);
        }
    }
}

Position Scene::getRandomPosition() {
    return Position(100 + 40 * objects.size(), 100); //TODO: add randomness
}

const std::vector<GameObject*>& Scene::getObjects() const {
    return objects;
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
    for (int i = 0; i < objects.size(); ++i)
    {
        Value player(kObjectType);
        player.AddMember("x", objects[i]->getX(), allc);
        player.AddMember("y", objects[i]->getY(), allc);
        player.AddMember("hp", ((Character*)objects[i])->getHp(), allc);
        player.AddMember("st", ((Character*)objects[i])->getStamina(), allc);
        player.AddMember("mhp", ((Character*)objects[i])->getMaxHp(), allc);
        player.AddMember("mst", ((Character*)objects[i])->getMaxStamina(), allc);
        player.AddMember("sid", (int)((Character*)objects[i])->getSpriteDirection(), allc);
        nick.SetString(((Character*)objects[i])->getID().data(), allc);
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

int Scene::test(lua_State *L) {
    std::cout << testIdx++ << std::endl;
}
