#include "Scene.h"
#include "GameObject.h"
//#include <windows.h> //TODO: delete include
#include <iomanip>

using namespace Engine;

//public methods

Scene::Scene() : land(true), wall(true) {
    height = Constants::SCENE_LENGTH;
    width = Constants::SCENE_HEIGHT;
    tiles.resize(height);
    for (auto row : tiles) {
        row.resize(width, &land);
    }
}

void Scene::move(GameObject *object, const Position &new_pos) {
    if (validPosition(new_pos)) {
        object->setPosition(new_pos);
    }
}

void Scene::move(GameObject *object, GameObject *target) {
    Position direction = findDirection(object, target);
    move(object, object->getPosition() + direction);
}

void Scene::attack(Character *c1, Character *c2) {
    if ((((GameObject*)c1)->getPosition() - ((GameObject*)c2)->getPosition()).abs() != 1.0) {
        move(c1, c2);
    }
    else if (c2->getHp() > 0) {
        c2->setHp(std::max(c2->getHp() - c1->getDamage(), 0));
    }
}

void Scene::update() {
    for (auto object : objects) {
        object->update();
    }
}

//void Scene::addObject(PGameObject obj) {
//    objects.push_back(obj);
//}

void Scene::addObject(GameObject *obj) {
    objects.push_back(obj);
}

void Scene::print() {
//    for (auto object : objects) {
//        std::cout << object->tmpLuaName << "\nPosition: " << object->getPosition().getX() << ' ' <<
//                     object->getPosition().getY() << "\nHp: " << ((Character*)object)->getHp() << std::endl;
//    }
    /* windows.h
    COORD coord;
    coord.X = 0;
    coord.Y = 0;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
    */
    //system("CLS");
    for (auto object : objects) {
        std::cout << object->tmpLuaName << "\nHp: " << std::setw(3) << std::right << ((Character*)object)->getHp() << std::endl;
    }
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
    return pos.getX() >= 0 &&
           pos.getY() >= 0 &&
           pos.getX() <= width &&
           pos.getY() <= height;
}

//int Scene::test(lua_State *L) {
//    Scene** Pscene = (Scene**)luaL_checkudata(L, 1, "SceneMetaTable");
//    int x = luaL_checkinteger(L, 2);
//    std::cout << x << " - passed by lua" << std::endl;
//    return 0;
//}

int Scene::getObjects(lua_State *L) {
    Scene* Pscene = *(Scene**)luaL_checkudata(L, 1, "SceneMetaTable");
    lua_newtable(L);
    //std::cout << "Table created" << std::endl;
    int i = 1;
    for (auto object : Pscene->objects) {
        //std::cout << i << " added to table" << std::endl;
        //lua_pushinteger(L, i++);
        ((Character*)object)->luaPush(L);
        lua_rawseti(L, -2, i++);
    }
    return 1;
}

Position Scene::findDirection(GameObject *from, GameObject *to) {
    Position fromPos = from->getPosition();
    Position toPos = to->getPosition();
    int diff;
    if ((diff = toPos.getX() - fromPos.getX())) {
        return Position(diff/abs(diff), 0);
    }
    else if ((diff = toPos.getY() - fromPos.getY())) {
        return Position(0, diff/abs(diff));
    }
}
