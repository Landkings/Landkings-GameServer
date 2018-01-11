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
    loadLuaCode(luaCode);
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
    if (lua_pcall(L, 1, 0, 0) != 0) {
        //std::cout << "Error running function f: " << lua_tostring(L, -1) << std::endl;
        //lua_tostring(L, -1);
        lua_pop(L, 1);
        return;
    }
    isStaminaHpRegenAvailable = false;
    maxStaminaTicks = 0;
    switch(action) {
    case Action::Move:
        move();
        break;
    case Action::Attack:
        attack();
        break;
    case Action::Block:
        block();
        break;
    case Action::Empty:
    default:;
        isStaminaHpRegenAvailable = true;
    }
}

void Character::setNextMoveTime() {
    nextMoveTime = scene->getTime() + (long long)moveCooldown;
}

void Character::luaPush(lua_State *state) {
    Character **Pcharacter = (Character**)lua_newuserdata(state, sizeof(Character*));
    //lua_pushlightuserdata(state, this);
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
    int currentDamage = 0;
    switch (attackType) {
    case AttackType::Fast:
        currentDamage = damage;
        nextAttackTime = scene->getTime() + attackCooldown;
        nextStaminaRegenTime = scene->getTime() + attackCooldown / 2;
        loseStamina(25);
        break;
    case AttackType::Strong:
        currentDamage = damage * 2;
        nextAttackTime = scene->getTime() + attackCooldown * 2;
        nextStaminaRegenTime = scene->getTime() + attackCooldown;
        loseStamina(50);
        break;
    }
    if (!target->isBlocking() || target->getBlockDirection() != attackDirection)
        target->takeDamage(currentDamage);
    else
        target->block(currentDamage * 2); //maybe do not multiply by 2
}

void Character::move(Position newPos) {
    Position delta = position - newPos; //TODO: refactor later!
    if (delta.getX() > 0)
        spriteDirection = SpriteDirection::Left;
    else if (delta.getX() < 0)
        spriteDirection = SpriteDirection::Right;
    else if (delta.getY() > 0)
        spriteDirection = SpriteDirection::Up;
    else if (delta.getY() < 0)
        spriteDirection = SpriteDirection::Down;

    position = newPos;
    switch(movementType) {
    case MovementType::Sprint:
       nextMoveTime = scene->getTime() + moveCooldown / 2;
       loseStamina(2);
       break;
    case MovementType::Default:
       nextMoveTime = scene->getTime() + moveCooldown;
       loseStamina(1);
       break;
    }
}

void Character::takeDamage(int amount) {
    if (amount > 0) {
        hitPoints -= amount;
        isStaminaHpRegenAvailable = false;
        maxStaminaTicks = 0;
    }
}

void Character::loadLuaCode(std::string luaCode) {
    closeLuaState();
    initLuaState();
    if (luaL_loadstring(L, luaCode.c_str()) || lua_pcall(L, 0, 0, 0)) {
        std::cout << "Error loading script: " << lua_tostring(L, -1) << std::endl;
    }
}

void Character::loseStamina(int amount) {
    stamina -= amount;
    if (stamina < 0) {
        takeDamage(-stamina);
        stamina = 0;
    }
}

void Character::gainStamina(int amount) {
    stamina += amount;
    if (stamina > maxStamina) {
        if (isStaminaHpRegenAvailable)
            gainHp(stamina - maxStamina);
        stamina = maxStamina;
    }
}

void Character::gainDefaultStamina() {
    if (!isStaminaHpRegenAvailable && stamina == maxStamina && ++maxStaminaTicks >= maxStaminaTicksRequirement)
        isStaminaHpRegenAvailable = true;
    gainStamina(1);
    nextStaminaRegenTime = scene->getTime() + moveCooldown; //maybe replace movecooldown
}

void Character::gainHp(int amount) {
    hitPoints = std::min(hitPoints + amount, maxHitPoints);
}

void Character::block(int amount) {
    loseStamina(amount);
}

Character::~Character() {
    closeLuaState();
}

void Character::attack() {
    if (target) {
        scene->attack(this, (Character*)target);
    }
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
    Character* Pcharacter = *((Character**)luaL_checkudata(state, 1, "CharacterMetaTable"));
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

int Character::luaSetAttackDirection(lua_State *state) {
    attackDirection = (AttackDirection)luaL_checkinteger(state, 1);
    return 0;
}

int Character::luaSetBlockDirection(lua_State *state) {
    blockDirection = (AttackDirection)luaL_checkinteger(state, 1);
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
    maxHitPoints = 100;
    hitPoints = maxHitPoints;
    damage = 10;    //TODO: replace with constants
    speed = 5;
    nextMoveTime = 0;
    nextAttackTime = 0;
    nextStaminaRegenTime = 0;
    moveCooldown = 16;
    attackCooldown = 400; //160
    maxStamina = 100;
    attackRange = 30;
    visionRange = 500;
    stamina = maxStamina;
    attackDirection = AttackDirection::Torso;
    blockDirection = AttackDirection::Torso;
    spriteDirection = SpriteDirection::Down;
    maxStaminaTicks = 0;
    maxStaminaTicksRequirement = attackCooldown + 100;
    isStaminaHpRegenAvailable = true;
    initLuaState();
}

void Character::closeLuaState() {
    lua_close(L);
}

void Character::initLuaState() {
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

    lua_pushstring(L, "AttackDirection");
    lua_newtable(L);
    ATTACK_DIRECTION
    lua_settable(L, -3);

    lua_pushstring(L, "BlockDirection");
    lua_newtable(L);
    ATTACK_DIRECTION
    lua_settable(L, -3);
#undef E
    //lua_pop(L, 1);

    luaL_openlibs(L);
    luaL_Reg characterMethods[] = {
        "setAction",          gloablDispatch<Character, &Character::luaSetAction>,
        "getAction",          gloablDispatch<Character, &Character::luaGetAction>,
        "setDirection",       gloablDispatch<Character, &Character::luaSetDirection>,
        "getDirection",       gloablDispatch<Character, &Character::luaGetDirection>,
        "setTarget",          gloablDispatch<Character, &Character::luaSetTarget>,
        "getTarget",          gloablDispatch<Character, &Character::luaGetTarget>,
        "getPosition",        gloablDispatch<Character, &Character::luaGetPosition>,
        "getStamina",         gloablDispatch<Character, &Character::luaGetStamina>,
        "getHp",              gloablDispatch<Character, &Character::luaGetHp>,
        "setAttackType",      gloablDispatch<Character, &Character::luaSetAttackType>,
        "setMovementType",    gloablDispatch<Character, &Character::luaSetMovementType>,
        "getMovementType",    gloablDispatch<Character, &Character::luaGetMovementType>,
        "setAttackDirection", gloablDispatch<Character, &Character::luaSetAttackDirection>,
        "setBlockDirection",  gloablDispatch<Character, &Character::luaSetBlockDirection>,
        "getMe",              gloablDispatch<Character, &Character::luaGetMe>,
        "write",              gloablDispatch<Character, &Character::write>,
        nullptr, nullptr
    };
    luaL_setfuncs(L, characterMethods, 0);
    scene->luaReg(L);
}

void Character::block() {
    if (scene->getTime() >= nextAttackTime) { //TODO: place this check in scene
        //loseStamina(10); //TODO: change
        nextAttackTime = scene->getTime() + attackCooldown;
    }
}
