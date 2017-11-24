#include "GameObject.h"

using namespace Engine;

// public methods

GameObject::GameObject(Scene *scene, Position pos, std::string tmpLuaName, HitBox hbox) : scene(scene), position(pos), tmpLuaName(tmpLuaName), hbox(hbox) {}

Character::Character(Scene *scene, Position pos, std::string tmpLuaName, HitBox hbox) :
    GameObject(scene, pos, tmpLuaName, hbox),
    target(nullptr),
    action(Action::Empty),
    direction(Direction::Unknown){
    hitPoints = 100;
    damage = 10;    //TODO: replace with constants
    L = luaL_newstate();
    *static_cast<Character**>(lua_getextraspace(L)) = this;

    lua_pushglobaltable(L); //TODO: replace global environment with a safe environment
#define E LUA_ENUM_HELPER
    // Add Action Enum
    lua_pushstring(L, "Action");
    lua_newtable(L);
    ACTION
    lua_settable(L, -3);

    //Add Direction Enum
    lua_pushstring(L, "Direction");
    lua_newtable(L);
    DIRECTION
    lua_settable(L, -3);
#undef E
    //lua_pop(L, 1);

    luaL_openlibs(L);
    //lua_pushglobaltable(L); //TODO: replace with a safe environment
    luaL_Reg characterMethods[] = {
        "setAction",    dispatch<Character, setAction>,
        "getAction",    dispatch<Character, getAction>,
        "setDirection", dispatch<Character, setDirection>,
        "getDirection", dispatch<Character, getDirection>,
        "setTarget",    dispatch<Character, setTarget>,
        "getTarget",    dispatch<Character, getTarget>,
        "getPosition",  dispatch<Character, getPosition>,
        "write",        dispatch<Character, write>,
        nullptr, nullptr
    };
    luaL_setfuncs(L, characterMethods, 0);
    scene->luaReg(L);
    if (luaL_loadfile(L, tmpLuaName.c_str()) || lua_pcall(L, 0, 0, 0)) {
        printf("Error loading script\n");
    }
}

void Character::update() {
    int t = lua_getglobal(L, "move"); //TODO: replace global environment with a safe environment
    scene->luaReg(L);
    lua_pcall(L, 1, 0, 0);
    switch(action) {
    case Action::Move:
        move();
        break;
    case Action::Attack:
        attack();
        break;
    case Action::Empty:
    default:;
    }
}

void Character::luaPush(lua_State *state) {
    Character **Pcharacter = (Character**)lua_newuserdata(state, sizeof(Character*));
    *Pcharacter = this;
    if (luaL_newmetatable(state, "CharacterMetaTable")) {
        lua_pushvalue(state, -1);
        lua_setfield(state, -2, "__index");

        luaL_Reg characterMethods[] = {
            "getPosition", dispatch<Character, getObjectPosition>,
            "test",        dispatch<Character, test>,
            nullptr, nullptr
        };
        luaL_setfuncs(state, characterMethods, 0);
    }
    lua_setmetatable(state, -2);
}

// private methods

void Character::move() {
    if (target) {
        scene->move(this, (GameObject*)target);
    }
    else if (direction != Direction::Unknown) {
        scene->move(this, position + directions[(int)direction]);
    }
}

void Character::attack() {
    scene->attack(this, (Character*)target);
}

// private methods

int Character::write(lua_State *state) {
    const char* msg = luaL_checkstring(state, 1);
    std::cout << msg << std::endl;
    return 0;
}

int Character::setAction(lua_State *state) {
    action = (Action)luaL_checkinteger(state, 1);
    return 0;
}

int Character::getAction(lua_State *state) {
    lua_pushstring(state, actionStrings[action].c_str());
    return 1;
}

int Character::setDirection(lua_State *state) {
    direction = (Direction)luaL_checkinteger(state, 1);
    return 0;
}

int Character::getDirection(lua_State *state) {
    lua_pushstring(state, directionStrings[direction].c_str());
    return 1;
}

int Character::setTarget(lua_State *state) {
    Character* Pcharacter = *(Character**)luaL_checkudata(state, 1, "CharacterMetaTable");
    if (Pcharacter != this) {
        target = Pcharacter;
    }
    else {
        target = nullptr;
    }
    return 0;
}

int Character::getTarget(lua_State *state) {
    if (target) {
        ((Character*)target)->luaPush(state);
    }
    else {
        lua_pushnil(state);
    }
    return 1;
}

int Character::getPosition(lua_State *state) {
    position.luaPush(state);
    return 1;
}

int Character::getObjectPosition(lua_State *state) {
    Character* Pcharacter = *(Character**)luaL_checkudata(state, 1, "CharacterMetaTable");
    Pcharacter->position.luaPush(state);
    return 1;
}

int Character::test(lua_State *stata) {
    std::cout << "Function called" << std::endl;
    return 0;
}
