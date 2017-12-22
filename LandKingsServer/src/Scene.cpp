#include "Scene.h"
#include "GameObject.h"
#include <iomanip>

using namespace Engine;
using namespace boost::property_tree;

//public methods

Scene::Scene() : land(true, 1), wall(false, 2) {
    height = Constants::SCENE_HEIGHT  / Constants::TILE_HEIGHT;
    width = Constants::SCENE_WIDTH / Constants::TILE_WIDTH;
    time = 0;
    tiles.resize(height);
    for (auto& row : tiles) {
        row.resize(width, &land);
    }
//    tiles[8][13] =  &wall;
//    tiles[10][13] =  &wall;
}

void Scene::move(GameObject *object, const Position &new_pos) {
    if (((Character*)object)->getNextMoveTime() <= time && validPosition(new_pos)) {
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
    Position newPos = c1->getPosition() + findDirection(c1, c2);
    if (!isCollide(newPos, c1->getHitbox(), c2->getPosition(), c2->getHitbox())) {
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
        object->update();
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
    *Pscene = this;
    if (luaL_newmetatable(L, "SceneMetaTable")) {
        lua_pushvalue(L, -1);
        lua_setfield(L, -2, "__index");

        luaL_Reg ScannerMethods[] = {
            "getObjects", dispatch<Scene, &Scene::getObjects>,
            nullptr, nullptr
        };
        luaL_setfuncs(L, ScannerMethods, 0);
    }
    lua_setmetatable(L, -2);
    //lua_setglobal(L, "Scene");
}

//private methods

bool Scene::validPosition(const Position & pos) {
    return pos.getX() >= 0 && //TODO add hitboxes
           pos.getY() >= 0 &&
           pos.getX() <= Constants::SCENE_WIDTH &&
           pos.getY() <= Constants::SCENE_HEIGHT;
}

int Scene::getObjects(lua_State *L) {
    Scene* Pscene = *(Scene**)luaL_checkudata(L, 1, "SceneMetaTable");
    lua_newtable(L);
    //std::cout << "Table created" << std::endl;
    int i = 1;
    for (auto& object : Pscene->objects) {
        //std::cout << i << " added to table" << std::endl;
        //lua_pushinteger(L, i++);
        ((Character*)object)->luaPush(L);
        lua_rawseti(L, -2, i++);
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
            players.erase((Character*)(objects[i])->getID()); //comment to respawn
            delete ((Character*)(objects[i]));
            objects.erase(objects.begin() + i);
        }
    }
}

Position Scene::getRandomPosition() {
    return Position(100, 100); //TODO: add randomness
}

const std::vector<GameObject*>& Scene::getObjects() const {
    return objects;
}

std::string Scene::getObjectsJSON() {
    ptree objectsJson;
    ptree playersJson;
    objectsMutex.lock();    
    for (int i = 0; i < objects.size(); ++i) {
        ptree ptObject;
        ptObject.put("x", objects[i]->getX());
        ptObject.put("y", objects[i]->getY());
        ptObject.put("hp", ((Character*)objects[i])->getHp());
        ptObject.put("stamina", ((Character*)objects[i])->getStamina());
        ptObject.put("id", ((Character*)objects[i])->getID());
        playersJson.push_back(make_pair("", ptObject));
        //playersJson.put_child(objects[i], ptObject);
    }
    objectsMutex.unlock();
    objectsJson.put<std::string>("messageType", "loadObjects");
    objectsJson.put_child("players", playersJson);
    std::stringstream ss;
    json_parser::write_json(ss, objectsJson);
    return ss.str();
}

std::string Scene::getTileMapJSON() {
    //engine.waitForMutex(_engine.sceneMutex);
    ptree result, mapJson;
    int height = tiles.size(), width = tiles[0].size();
    result.put<std::string>("messageType", "loadMap");
    result.put<int>("width", width);
    result.put<int>("height", height);
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            ptree val;
            val.put("", tiles[i][j]->getIdx());
            mapJson.push_back(make_pair("", val));
        }
    }
    result.put_child("tileMap", mapJson);
    std::stringstream ss;
    json_parser::write_json(ss, result);
    return ss.str();
    //socket->send(ss.str().data(), ss.str().length(), TEXT);
    //_engine.sceneMutex.unlock();
}


