#include "GameObject.h"

using namespace Engine;

// public methods

GameObject::GameObject(Scene *scene, Position pos, std::string name, HitBox hbox) : scene(scene), position(pos), hbox(hbox), name(name) {}

Character::Character(Scene *scene, Position pos, std::string tmpLuaName, HitBox hbox) :
    GameObject(scene, pos, "", hbox) {
    init();
    if (luaL_loadfile(L, tmpLuaName.c_str()) || lua_pcall(L, 0, 0, 0)) {
        printf("Error loading script\n");
    }
}

Character::Character(Scene *scene, std::string luaCode, std::string name, Position pos) :
    GameObject(scene, pos, name, HitBox(20, 20)) {
    init();
    if (luaL_loadstring(L, luaCode.c_str()) || lua_pcall(L, 0, 0, 0)) {
        printf("Error loading script\n");
    }
}

void Character::move() {
    if (target) {
        scene->move(this, (GameObject*)target);
    }
    else if (direction != Direction::Unknown) {
        scene->move(this, position + directions[(int)direction]);
    }
}

int Character::write(lua_State *state) {
    const char* msg = luaL_checkstring(state, 1);
    std::cout << msg << std::endl;
    return 0;
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

void Character::setNextMoveTime() {
    nextMoveTime = scene->getTime() + (long long)moveCooldown;
}

void Character::luaPush(lua_State *state) {
    Character **Pcharacter = (Character**)lua_newuserdata(state, sizeof(Character*));
    *Pcharacter = this;
    if (luaL_newmetatable(state, "CharacterMetaTable")) {
        lua_pushvalue(state, -1);
        lua_setfield(state, -2, "__index");

        luaL_Reg characterMethods[] = {
            "getPosition", dispatch<Character, &Character::luaGetObjectPosition>,
            "test",        dispatch<Character, &Character::test>,
            nullptr, nullptr
        };
        luaL_setfuncs(state, characterMethods, 0);
    }
    lua_setmetatable(state, -2);
}

void Character::attack(Character *target) {
    switch (attackType) {
    case AttackType::Fast:
        target->takeDamage(damage);
        nextAttackTime = scene->getTime() + attackCooldown;
        break;
    case AttackType::Strong:
        target->takeDamage(damage * 2);
        nextAttackTime = scene->getTime() + attackCooldown * 2;
        break;
    }
}

void Character::move(Position newPos) {
    position = newPos;
    switch(movementType) {
    case MovementType::Sprint:
       nextMoveTime = scene->getTime() + moveCooldown / 2;
       break;
    case MovementType::Default:
        nextMoveTime = scene->getTime() + moveCooldown;
       break;
    }
    //setNextMoveTime();
}

void Character::takeDamage(int amount) {
    if (amount > 0)
        hitPoints -= amount;
}

Character::~Character() {
    lua_close(L);
}

void Character::attack() {
    if (target)
        scene->attack(this, (Character*)target);
}

// private methods

int Character::luaSetAction(lua_State *state) {
    action = (Action)luaL_checkinteger(state, 1);
    return 0;
}

int Character::luaGetAction(lua_State *state) {
    //lua_pushstring(state, actionStrings[action].c_str());
    lua_pushinteger(state, (int)action);
    return 1;
}

int Character::luaSetDirection(lua_State *state) {
    direction = (Direction)luaL_checkinteger(state, 1);
    return 0;
}

int Character::luaGetDirection(lua_State *state) {
    //lua_pushstring(state, directionStrings[direction].c_str());
    lua_pushinteger(state, (int)direction);
    return 1;
}

int Character::luaSetTarget(lua_State *state) {
    Character* Pcharacter = *(Character**)luaL_checkudata(state, 1, "CharacterMetaTable");
    if (Pcharacter != this) {
        target = Pcharacter;
    }
    else {
        target = nullptr;
    }
    return 0;
}

int Character::luaGetTarget(lua_State *state) {
    if (target) {
        ((Character*)target)->luaPush(state);
    }
    else {
        lua_pushnil(state);
    }
    return 1;
}

int Character::luaGetPosition(lua_State *state) {
    position.luaPush(state);
    return 1;
}

int Character::luaGetObjectPosition(lua_State *state) {
    Character* Pcharacter = *(Character**)luaL_checkudata(state, 1, "CharacterMetaTable");
    Pcharacter->position.luaPush(state);
    return 1;
}

int Character::luaGetStamina(lua_State *state) {
    lua_pushinteger(state, stamina);
    return 1;
}

int Character::luaGetHp(lua_State *state) {
    lua_pushinteger(state, hitPoints);
    return 1;
}

int Character::luaSetAttackType(lua_State *state) {
    attackType = (AttackType)luaL_checkinteger(state, 1);
    return 0;
}

int Character::luaSetMovementType(lua_State *state) {
    movementType = (MovementType)luaL_checkinteger(state, 1);
    return 0;
}

int Character::luaGetMovementType(lua_State *state) {
    lua_pushinteger(state, (int)movementType);
    return 1;
}

int Character::luaGetMe(lua_State *state) {
    this->luaPush(state);
    return 1;
}

int Character::test(lua_State *stata) {
    std::cout << "Function called" << std::endl;
    return 0;
}

void Character::init() {
    target = nullptr;
    action = Action::Empty;
    direction = Direction::Unknown;
    attackType = AttackType::Fast;
    movementType = MovementType::Default;
    hitPoints = 100;
    damage = 10;    //TODO: replace with constants
    speed = 5;
    nextMoveTime = 0;
    nextAttackTime = 0;
    moveCooldown = 16;
    attackCooldown = 400;
    stamina = 100;

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

    //Add AttackType Enum
    lua_pushstring(L, "AttackType");
    lua_newtable(L);
    ATTACK_TYPE
    lua_settable(L, -3);

    lua_pushstring(L, "MovementType");
    lua_newtable(L);
    MOVEMENT_TYPE
    lua_settable(L, -3);

#undef E
    //lua_pop(L, 1);

    luaL_openlibs(L);
    //lua_pushglobaltable(L); //TODO: replace with a safe environment
    luaL_Reg characterMethods[] = {
        "setAction",       dispatch<Character, &Character::luaSetAction>,
        "getAction",       dispatch<Character, &Character::luaGetAction>,
        "setDirection",    dispatch<Character, &Character::luaSetDirection>,
        "getDirection",    dispatch<Character, &Character::luaGetDirection>,
        "setTarget",       dispatch<Character, &Character::luaSetTarget>,
        "getTarget",       dispatch<Character, &Character::luaGetTarget>,
        "getPosition",     dispatch<Character, &Character::luaGetPosition>,
        "getStamina",      dispatch<Character, &Character::luaGetStamina>,
        "getHp",           dispatch<Character, &Character::luaGetHp>,
        "setAttackType",   dispatch<Character, &Character::luaSetAttackType>,
        "setMovementType", dispatch<Character, &Character::luaSetMovementType>,
        "getMovementType", dispatch<Character, &Character::luaGetMovementType>,
        "getMe",           dispatch<Character, &Character::luaGetMe>,
        "write",           dispatch<Character, &Character::write>,
        nullptr, nullptr
    };
    luaL_setfuncs(L, characterMethods, 0);
    scene->luaReg(L);
}
