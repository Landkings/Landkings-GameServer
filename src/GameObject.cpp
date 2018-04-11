#include "GameObject.h"

using namespace Engine;

// public methods

GameObject::GameObject(Scene *scene, ObjectType type, Vec2i pos, std::string name, HitBox hbox) :
    scene(scene), position(pos), hbox(hbox), name(name), type(type) {}

Character::Character(Scene *scene, Vec2i pos, std::string tmpLuaName, HitBox hbox, ObjectType type) :
    GameObject(scene, type, pos, "", hbox),
    inventory(20) {
    init();
    if (luaL_loadfile(L, tmpLuaName.c_str()) || lua_pcall(L, 0, 0, 0)) {
        printf("Error loading script\n");
    }
}

Character::Character(Scene *scene, std::string luaCode, std::string name, Vec2i pos, ObjectType type) :
    GameObject(scene, type, pos, name, HitBox(32, 23)),
    inventory(20) {
    init();
    loadLuaCode(luaCode);
}

void Character::update() {
    if (!usingAction) {
        int t = lua_getglobal(L, "move"); //TODO: replace global environment with a safe environment
        scene->luaPush(L);
        lua_KContext ctx;
        //lua_
        int res = lua_pcallk(L, 1, 0, 0, ctx, &dispatchContinuation<Character, &Character::luaContinuationTest>);
        if (res != LUA_OK) {
            //std::cout << "Status: " << lua_status(L) << std::endl;
            //std::cout << "Error running function f: " << lua_tostring(L, -1) << std::endl;
            //lua_tostring(L, -1);
            if (res == LUA_YIELD)
                ;
                //std::cout << "We yielded" << std::endl;
            else {
                //std::cout << "Something different occured. res = " << res << std::endl;
            }
            lua_pop(L, 1);
            return;
        }
    }
    bool wasUsingAction = usingAction;
    isStaminaRegenAvailable = true;
    //isStaminaHpRegenAvailable = false;
    //maxStaminaTicks = 0;
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
        //isStaminaHpRegenAvailable = true;
    }
    if (wasUsingAction)
        usingAction = false;
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
            "getObjectType", dispatch<Character, &Character::luaGetObjectType>,
            //"test",        dispatch<Character, &Character::test>,
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
        nextAttackTime = scene->getTime() + getAttackCooldown() / 2;
        break;
    case AttackType::Strong:
        currentDamage = damage * 2;
        nextAttackTime = scene->getTime() + getAttackCooldown() * 2 / 2;
        break;
    }
    if (usingAction) {
        if (!target->blocking() || target->getBlockDirection() != attackDirection)
            target->takeDamage(currentDamage);
        else
            target->block(currentDamage / 2);
    }
    else {
        usingAction = true;
    }
}

void Character::move(Vec2i newPos) {
    Vec2i delta = position - newPos; //TODO: refactor later!
    if (delta.getX() > 0)
        spriteDirection = SpriteDirection::Left;
    else if (delta.getX() < 0)
        spriteDirection = SpriteDirection::Right;
    else if (delta.getY() > 0)
        spriteDirection = SpriteDirection::Up;
    else if (delta.getY() < 0)
        spriteDirection = SpriteDirection::Down;

    position = newPos;
    isStaminaRegenAvailable = false;
    maxStaminaTicks = 0;
    switch(movementType) {
    case MovementType::Sprint:
       nextMoveTime = scene->getTime() + getMoveCooldown() / 2;
       loseStamina(getSprintStaminaCost());
       break;
    case MovementType::Default:
       nextMoveTime = scene->getTime() + getMoveCooldown();
       //loseStamina(getMoveStaminaCost());
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
    if (L != nullptr)
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

    if (isStaminaRegenAvailable) {
        gainStamina(getMoveStaminaCost()); //maybe replace stamina cost
        nextStaminaRegenTime = scene->getTime() + moveCooldown; //maybe replace movecooldown
    }
}

void Character::gainHp(int amount) {
    hitPoints = std::min(hitPoints + amount, maxHitPoints);
}

void Character::block(int amount) {
    loseStamina(amount);
}

void Character::takeItem(Item *item) {
    item->use(this);
    //inventory.addItem(item);
}

void Character::useItem(Item *item) {
    item->use(this);
}

void Character::gainExp(int amount) {
    currentExp += amount;
    while (currentExp >= nextLevelExp) {
        hitPoints = maxHitPoints;
        currentExp -= nextLevelExp;
        ++level;
        ++skillPoints;
        nextLevelExp = level * 500;
    }
}

void Character::payAttackCost() {
    switch (attackType) {
    case AttackType::Fast:
        nextStaminaRegenTime = scene->getTime() + getAttackCooldown() / 2;
        loseStamina(getAttackStaminaCost());
        break;
    case AttackType::Strong:
        nextStaminaRegenTime = scene->getTime() + getAttackCooldown();
        loseStamina(getAttackStaminaCost() * 2);
        break;
    }
}

int Character::getExpValue() {
    return 250 + level * 100; //TODO: replace with constants
}

GameObject *Character::clone() {
    Character* newCharacter = new Character(*this);
    newCharacter->L = nullptr;
    return newCharacter;
}

Character::~Character() {
    closeLuaState();
}

// private methods

void Character::init() {
    target = nullptr;
    action = Action::Empty;
    direction = Direction::Unknown;
    attackType = AttackType::Fast;
    movementType = MovementType::Default;
    maxHitPoints = 50;
    hitPoints = maxHitPoints;
    damage = 4;    //TODO: replace with constants
    speed = 2;
    nextMoveTime = 0;
    nextAttackTime = 0;
    nextStaminaRegenTime = 0;
    moveCooldown = 40;
    attackCooldown = 500; //160
    maxStamina = 100;
    attackRange = 45;
    visionRange = 100;
    stamina = maxStamina;
    attackDirection = AttackDirection::Torso;
    blockDirection = AttackDirection::Torso;
    spriteDirection = SpriteDirection::Down;
    maxStaminaTicks = 0;
    maxStaminaTicksRequirement = attackCooldown * 10;
    isStaminaHpRegenAvailable = true;
    isStaminaRegenAvailable = true;
    attackStaminaCost = 0;
    moveStaminaCost = 0;
    sprintStaminaCost = 0;
    blockStaminaCost = 0;
    level = 1;
    nextLevelExp = 500;
    skillPoints = 0;
    currentExp = 0;
    usingAction = false;
    for (int i = 0; i < (int)Parameters::Size; ++i)
        parameters[(Parameters)i] = 0;
    initLuaState();
}

void Player::init() {
    target = nullptr;
    action = Action::Empty;
    direction = Direction::Unknown;
    attackType = AttackType::Fast;
    movementType = MovementType::Default;
    maxHitPoints = 100;
    hitPoints = maxHitPoints;
    damage = 10;    //TODO: replace with constants
    speed = 3;
    nextMoveTime = 0;
    nextAttackTime = 0;
    nextStaminaRegenTime = 0;
    moveCooldown = 25;
    attackCooldown = 400; //160
    maxStamina = 400;
    attackRange = 45;
    visionRange = 150;
    stamina = maxStamina;
    attackDirection = AttackDirection::Torso;
    blockDirection = AttackDirection::Torso;
    spriteDirection = SpriteDirection::Down;
    maxStaminaTicks = 0;
    maxStaminaTicksRequirement = attackCooldown * 10;
    isStaminaHpRegenAvailable = true;
    isStaminaRegenAvailable = true;
    attackStaminaCost = 25;
    moveStaminaCost = 1;
    sprintStaminaCost = 2;
    blockStaminaCost = 25;
    level = 1;
    nextLevelExp = 500;
    skillPoints = 0;
    currentExp = 0;
    usingAction = false;
    for (int i = 0; i < (int)Parameters::Size; ++i)
        parameters[(Parameters)i] = 0;
    initLuaState();
}

Player::Player(Scene *scene, Vec2i pos, std::string tmpLuaName, HitBox hbox) :
        Character(scene, pos, tmpLuaName, hbox, ObjectType::Player) {
    init();
}

Player::Player(Scene *scene, std::string luaCode, std::string name, Vec2i pos) :
        Character(scene, luaCode, name, pos, ObjectType::Player) {
    init();
}

void Character::closeLuaState() {
    lua_close(L);
}

void Character::initLuaState() {
    L = luaL_newstate();
    *static_cast<Character**>(lua_getextraspace(L)) = this;
    //lua_Hook h(&dispatchHook<Character, &Character::luaCountHook>);
    lua_sethook(L, dispatchHook<Character, &Character::luaCountHook>, LUA_MASKCOUNT, 100000); //todo tune up

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

    lua_pushstring(L, "Parameters");
    lua_newtable(L);
    PARAMETERS
    lua_settable(L, -3);

    lua_pushstring(L, "ObjectType");
    lua_newtable(L);
    OBJECT_TYPE
    lua_settable(L, -3);
#undef E
    //lua_pop(L, 1);

    luaL_openlibs(L);
    luaL_Reg characterMethods[] = {
        "setAction",               globalDispatch<Character, &Character::luaSetAction>,
        "getAction",               globalDispatch<Character, &Character::luaGetAction>,         //?
        "setDirection",            globalDispatch<Character, &Character::luaSetDirection>,
        "getDirection",            globalDispatch<Character, &Character::luaGetDirection>,      //?
        "setTarget",               globalDispatch<Character, &Character::luaSetTarget>,
        "getTarget",               globalDispatch<Character, &Character::luaGetTarget>,
        "getPosition",             globalDispatch<Character, &Character::luaGetPosition>,
        "getStamina",              globalDispatch<Character, &Character::luaGetStamina>,
        "getHp",                   globalDispatch<Character, &Character::luaGetHp>,
        "setAttackType",           globalDispatch<Character, &Character::luaSetAttackType>,
        "setMovementType",         globalDispatch<Character, &Character::luaSetMovementType>,
        "getMovementType",         globalDispatch<Character, &Character::luaGetMovementType>,
        "setAttackDirection",      globalDispatch<Character, &Character::luaSetAttackDirection>,
        "setBlockDirection",       globalDispatch<Character, &Character::luaSetBlockDirection>,
        "getMe",                   globalDispatch<Character, &Character::luaGetMe>,             //?
        "getAttackStaminaCost",    globalDispatch<Character, &Character::luaGetAttackStaminaCost>,
        "getMoveStaminaCost",      globalDispatch<Character, &Character::luaGetMoveStaminaCost>,
        "getBlockStaminaCost",     globalDispatch<Character, &Character::luaGetBlockStaminaCost>,
        "getSprintStaminaCost",    globalDispatch<Character, &Character::luaGetSprintStaminaCost>,
        "levelUp",                 globalDispatch<Character, &Character::luaLevelUp>,
        "getCurrentExp",           globalDispatch<Character, &Character::luaGetCurrentExp>,
        "getNextLevelExp",         globalDispatch<Character, &Character::luaGetNextLevelExp>,
        "getAvailableSkillPoints", globalDispatch<Character, &Character::luaGetAvailableSkillPoints>,
        "getParameterLevelUpCost", globalDispatch<Character, &Character::luaGetParameterLevelUpCost>,
        "canMove",                 globalDispatch<Character, &Character::luaCanMove>,
        "canAttack",               globalDispatch<Character, &Character::luaCanAttack>,
        nullptr, nullptr
    };
    luaL_setfuncs(L, characterMethods, 0);
    scene->luaPush(L);
}

void Character::attack() {
    if (target)
    if (target->getType() == ObjectType::Player ||
            target->getType() == ObjectType::NPC) {
        scene->attack(this, (Character*)target);
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


void Character::block() {
    if (scene->getTime() >= nextAttackTime) { //TODO: place this check in scene
        //loseStamina(10); //TODO: change
        nextAttackTime = scene->getTime() + attackCooldown;
    }
}

void Character::takeItem() {
   //if (target && (target->getType() == ObjectType::HealingItem || target->getType() == ObjectType::ExpItem)) { //TODO: replace
   //    scene->takeItem(this, item);
   //}
}

void HealingItem::use(Character *target) {
    target->gainHp(healAmount);
    ++consumedCharges;
}

GameObject *HealingItem::clone() {
    HealingItem* newHealingItem = new HealingItem(*this);//scene, position, hbox, healAmount, size);
    return newHealingItem;
}

//void Item::collect(Character *target) {
//    target->takeItem(this);
//    isInInventory = true;
//}

// ---------------------- LUA FUNCTIONS -----------------------

int Character::luaSetAction(lua_State *state) {
    int actionInt = luaL_checkinteger(state, 1);
    if (actionInt < (int)Action::Size) {
        action = (Action)actionInt;
    }
    else {
        //TODO: raise error
    }

    return 0;
}

int Character::luaGetAction(lua_State *state) {
    lua_pushinteger(state, (int)action);
    return 1;
}

int Character::luaSetDirection(lua_State *state) {
    int directionInt = luaL_checkinteger(state, 1);
    if (directionInt < (int)Direction::Size) {
        direction = (Direction)directionInt;
    }
    else {
        //TODO: raise error
    }
    return 0;
}

int Character::luaGetDirection(lua_State *state) {
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
    if (target)
        ((Character*)target)->luaPush(state);
    else
        lua_pushnil(state);
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
    int attackTypeInt = luaL_checkinteger(state, 1);
    if (attackTypeInt >= 0 && attackTypeInt < (int)AttackType::Size) {
        attackType = (AttackType)attackTypeInt;
    }
    return 0;
}

int Character::luaSetAttackDirection(lua_State *state) {
    int attackDirectionInt = luaL_checkinteger(state, 1);
    if (attackDirectionInt >= 0 && attackDirectionInt < (int)AttackDirection::Size) {
        attackDirection = (AttackDirection)attackDirectionInt;
    }
    else {
        //TODO: raise error
    }
    return 0;
}

int Character::luaSetBlockDirection(lua_State *state) {
    int blockDirectionInt = luaL_checkinteger(state, 1);
    if (blockDirectionInt >= 0 && blockDirectionInt < (int)AttackDirection::Size) {
        blockDirection = (AttackDirection)blockDirection;
    }
    else {
        //TODO: raise error
    }
    return 0;
}

int Character::luaSetMovementType(lua_State *state) {
    int movementTypeInt = luaL_checkinteger(state, 1);
    if (movementTypeInt < (int)MovementType::Size) {
        movementType = (MovementType)movementTypeInt;
    }
    else {
        //TODO: raise error
    }
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

int Character::luaGetAttackStaminaCost(lua_State *state) {
    lua_pushinteger(state, getAttackStaminaCost());
    return 1;
}

int Character::luaGetMoveStaminaCost(lua_State *state) {
    lua_pushinteger(state, getMoveStaminaCost());
    return 1;
}

int Character::luaGetBlockStaminaCost(lua_State *state) {
    lua_pushinteger(state, getBlockStaminaCost());
    return 1;
}

int Character::luaGetSprintStaminaCost(lua_State *state) {
    lua_pushinteger(state, getSprintStaminaCost());
    return 1;
}

int Character::luaLevelUp(lua_State *state) {
    int paramInt = luaL_checkinteger(state, 1);
    if (paramInt >= 0 && paramInt < (int)Parameters::Size) {
        Parameters param = (Parameters)paramInt;
        if (skillPoints >= parameters[param] + 1) {
            ++parameters[param];
            skillPoints -= parameters[param];
        }
    }
    else {
        //TODO: raise error
    }
    return 0;
}

int Character::luaGetAvailableSkillPoints(lua_State *state) {
    lua_pushinteger(state, skillPoints);
    return 1;
}

int Character::luaGetCurrentExp(lua_State *state) {
    lua_pushinteger(state, currentExp);
    return 1;
}

int Character::luaGetNextLevelExp(lua_State *state) {
    lua_pushinteger(state, nextLevelExp);
    return 1;
}

int Character::luaGetParameterLevel(lua_State *state) {
    int paramInt = luaL_checkinteger(state, 1);
    if (paramInt >= 0 && paramInt < (int)Parameters::Size) {
        Parameters param = (Parameters)paramInt;
        lua_pushinteger(state, parameters[param] + 1);
    }
    else  {
        //TODO: raise error
    }
    return 1;
}

int Character::luaGetParameterLevelUpCost(lua_State *state) {
    int paramInt = luaL_checkinteger(state, 1);
    if (paramInt >= 0 && paramInt < (int)Parameters::Size) {
        Parameters param = (Parameters)paramInt;
        lua_pushinteger(state, parameters[param] + 1);
    }
    else {
       //TODO: raise error
    }
    return 1;
}

int Character::luaCanMove(lua_State *state) {
    int directionInt = luaL_checkinteger(state, 1);
    Direction dir;
    if (directionInt < (int)Direction::Size) {
        dir = (Direction)directionInt;
    }
    else {
        //TODO: raise error
    }
    lua_pushboolean(state, scene->canMove(this, position + directions[(int)dir]));
    return 1;
}

int Character::luaUseItem(lua_State *state) {
    Item *item = *(Item**)luaL_checkudata(state, -1, "ItemMetaTable");
    //TOOD: if (...)
        useItem(item);
        return 0;
}

int Character::luaCanAttack(lua_State *state) {
    Character* target = (Character*)lua_touserdata(state, -1);
    lua_pushboolean(state, scene->canAttack(this, target));
    return 1;
}

int Character::luaGetObjectType(lua_State *state) {
    lua_pushinteger(state, (int)type);
    return 1;
}

void Character::luaCountHook(lua_State *state, lua_Debug *ar) {
    //lua_getinfo(L, ar);
    //std:: cout << "Is yieldable: " << lua_isyieldable(state) << std::endl;
    //lua_yield(state, 0);
    //lua_resume(L, L, 0);
    //std::cout << "Hook called" << std::endl;
    lua_error(state);
    //lua_resume(state, 0);
}

int Character::luaContinuationTest(lua_State *state, int status, lua_KContext ctx) {
    std::cout << "We are in continuation function" << std::endl;
}

void ExpItem::use(Character *target) {
    target->gainExp(expAmount);
    ++consumedCharges;
}

GameObject *ExpItem::clone() {
    ExpItem* expItem= new ExpItem(*this);
    return expItem;
}

void Item::luaPush(lua_State *state) {
    Item **Ptem= (Item**)lua_newuserdata(state, sizeof(Item*));
    *Ptem= this;
    if (luaL_newmetatable(state, "ItemMetaTable")) {
        lua_pushvalue(state, -1);
        lua_setfield(state, -2, "__index");

        luaL_Reg ItemsMethods[] = {
            "getObjectType", dispatch<Item, &Item::luaGetObjectType>,
            "getPosition", dispatch<Item, &Item::luaGetPosition>,
            nullptr, nullptr
        };
        luaL_setfuncs(state, ItemsMethods, 0);
    }
    lua_setmetatable(state, -2);
}

int Item::luaGetObjectType(lua_State *state) {
    lua_pushinteger(state, (int)type);
    return 1;
}

int Item::luaGetPosition(lua_State *state) {
    position.luaPush(state);
    return 1;
}
