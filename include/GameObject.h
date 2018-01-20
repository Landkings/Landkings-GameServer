#pragma once
#include <string>
#include <memory>
#include <functional>
#include <map>

#include "Scene.h"
#include "Vec2i.h"
#include "Hitbox.h"
#include "lua.hpp"
#include "LuaHelper.h"
#include "Inventory.h"

namespace Engine {

#define ACTION \
  E(Move, Move, Action) \
  E(Attack, Attack, Action) \
  E(Block, Block, Action) \
  E(Empty, Empty, Action)

#define DIRECTION \
  E(Up, Up, Direction) \
  E(Right, Right, Direction) \
  E(Down, Down, Direction) \
  E(Left, Left, Direction) \
  E(Unknown, Unknown, Direction)

#define ATTACK_TYPE \
  E(Fast, Fast, AttackType) \
  E(Strong, Strong, AttackType)

#define ATTACK_DIRECTION \
  E(Head, Head, AttackDirection) \
  E(Torso, Torso, AttackDirection) \
  E(Legs, Legs, AttackDirection)

#define MOVEMENT_TYPE \
  E(Default, Default, MovementType) \
  E(Sprint, Sprint, MovementType)

#define PARAMETERS \
    E(MovementSpeed, MovementSpeed, Parameters) \
    E(AttackSpeed, AttackSpeed, Parameters) \
    E(AttackRange, AttackRange, Parameters) \
    E(HitPoints, HitPoints, Parameters) \
    E(StaminaPoints, StaminaPoints, Parameters) \
    E(AttackDamage, AttackDamage, Parameters) \
    E(VisionRange, VisionRange, Parameters) \
    E(StaminaCostReduction, StaminaCostReduction, Parameters) \
    E(StaminaRegenFrequency, StaminaRegenFrequency, Parameters)

#define E C_ENUM_HELPER
enum class Action {
    ACTION
    Size
};

enum class Direction {
    DIRECTION
    Size
};

enum class AttackType {
    ATTACK_TYPE
    Size
};

enum class MovementType {
    MOVEMENT_TYPE
    Size
};

enum class AttackDirection {
    ATTACK_DIRECTION
    Size
};

enum class Parameters {
    PARAMETERS
    Size
};
#undef E

//#define E C_ENUM_TO_STRING_HELPER
//static std::map<Action, std::string> actionStrings = {
//    ACTION
//};
//
//static std::map<Direction, std::string> directionStrings = {
//    DIRECTION
//};
//#undef E

enum class SpriteDirection {
    Up = 0,
    Right = 1,
    Down = 2,
    Left = 3
};

const Vec2i directions[4] = {
        Vec2i(0, -1),
        Vec2i(1, 0),
        Vec2i(0, 1),
        Vec2i(-1, 0)
};

class GameObject {
public:
    GameObject(Scene* scene, Vec2i pos = Vec2i(), std::string name = "", HitBox hbox = HitBox());
    Vec2i getPosition() const { return position; }
    virtual void setPosition(Vec2i pos) { position = pos; }
    virtual void setName(std::string newName) { name = newName; }
    virtual HitBox getHitbox() const { return hbox; }
    virtual void setHitbox(HitBox& hitbox) { hbox = hitbox; }
    virtual std::string getName() { return name; }
    virtual int getX() const { return position.getX(); }
    virtual int getY() const { return position.getY(); }
    virtual int getWidth() const { return hbox.getWidth(); }
    virtual int getHeight() const { return hbox.getHeight(); }
    virtual std::string getID() const { return name; }
    virtual bool isPassable() { return false; }
    virtual GameObject* clone() = 0;
    virtual void luaPush(lua_State *state) = 0;
protected:
    Vec2i position;
    HitBox hbox;
    Scene *scene;
    std::string name; //?
};

class Item : public GameObject {
public:
    Item(Scene *scene, Vec2i pos, std::string name, HitBox hbox, int size, int maxCharges,
         int useCooldown, int staminaCost, int actionCooldown) :
        GameObject(scene, pos, name, hbox),
        maxCharges(maxCharges),
        consumedCharges(0),
        size(size),
        isInInventory(false),
        useCooldown(useCooldown),
        staminaCost(staminaCost),
        actionCooldown(actionCooldown) {
    }
    virtual void use(Character* target) = 0;
    virtual bool used() const { return consumedCharges == maxCharges; }
    virtual int chargesLeft() const { return maxCharges - consumedCharges ; }
    virtual int getSize() const { return size; }
    virtual int getUseCooldown() const { return useCooldown; }
    virtual int getStaminaCost() const { return staminaCost; }
    virtual int getActionCooldown() const { return actionCooldown; }
    //virtual void collect(Character* target);
    virtual bool inInventory() const { return isInInventory; }
protected:
    int maxCharges;
    int consumedCharges;
    int size;
    bool isInInventory;
    int useCooldown;
    int staminaCost;
    int actionCooldown;
};

class HealingItem : public Item {
public:
    HealingItem(Scene *scene, Vec2i pos, HitBox hbox, int amount, int size, int maxCharges,
                int useCooldown, int staminaCost, int actionCooldown) :
        Item(scene, pos, "HealingItem", hbox, size, maxCharges, useCooldown, staminaCost, actionCooldown),
        healAmount(amount) {
    }
    void use(Character *target) override;
    void luaPush(lua_State *state);
    GameObject *clone();
private:
    int healAmount;
};

class Character : public GameObject {
public:
    Character(Scene *scene, Vec2i pos = Vec2i(), std::string tmpLuaName = "", HitBox hbox = HitBox(20, 20));
    Character(Scene *scene, std::string luaCode, std::string name, Vec2i pos = Vec2i());
    //int write(lua_State *state);
    void update();
    void luaPush(lua_State *state) override;
    void attack(Character *target);
    void move(Vec2i newPos);
    void takeDamage(int amount);
    void loadLuaCode(std::string luaCode);
    bool blocking() const { return action == Action::Block; }
    void loseStamina(int amount);
    void gainStamina(int amount);
    void gainDefaultStamina();
    void gainHp(int amount);
    void block(int amount);
    void takeItem(Item *item);
    void useItem(Item *item);
    void gainExp(int amount);
    int getExpValue();
    GameObject* clone();
    ~Character();

    //getters and setters
    int getSpeed() const { return speed; }
    int getHp() const { return hitPoints; }
    void setHp(const int hp) { hitPoints = hp; }
    const int getMaxHp() { return maxHitPoints + 10 * parameters[Parameters::HitPoints]; } //TODO: replace with constant
    const int getStamina() { return stamina; }
    const int getMaxStamina() { return maxStamina + 10 * parameters[Parameters::StaminaPoints]; } //TODO: replace with constant
    const int getDamage() { return damage + 1 * parameters[Parameters::AttackDamage]; } //TODO: replace with constant
    void setDirection(const Direction dir) { direction = dir; }
    void setTarget (const GameObject *targ) { target = targ; }
    const int getMoveCooldown() { return moveCooldown - 1 * (bool)parameters[Parameters::MovementSpeed]; } //TODO: replace with constant
    const int getAttackCooldown() { return attackCooldown - 10 * parameters[Parameters::AttackSpeed]; } //TODO: replace with constant
    long long getNextMoveTime() const { return nextMoveTime; }    
    long long getNextAttackTime() const {return nextAttackTime; }
    const int getVisionRange() { return visionRange + 50 * parameters[Parameters::VisionRange]; }
    SpriteDirection getSpriteDirection() const { return spriteDirection; }
    bool isOnCooldown() const { return nextAttackTime > scene->getTime() || nextMoveTime > scene->getTime(); }
    const int getAttackRange() { return attackRange + 5 * parameters[Parameters::AttackRange]; } //TODO: replace with constants
    AttackType getAttackType() const { return attackType; }
    AttackDirection getBlockDirection() const { return blockDirection; }
    AttackDirection getAttackDirection() const { return attackDirection; }
    long long getNextStaminaRegenTime() const { return nextStaminaRegenTime; }
    int getAttackStaminaCost() const { return attackStaminaCost; }
    int getMoveStaminaCost() const { return moveStaminaCost; }
    int getBlockStaminaCost() const { return blockStaminaCost; }
    int getSprintStaminaCost() const { return sprintStaminaCost; }
protected:
    void init();
    void initLuaState();
    void closeLuaState();
    void move();
    void attack();
    void block();

    //lua functions
    int luaSetAction(lua_State *state);
    int luaGetAction(lua_State *state);
    int luaSetDirection(lua_State *state);
    int luaGetDirection(lua_State *state);
    int luaSetTarget(lua_State *state);
    int luaGetTarget(lua_State *state);
    int luaGetPosition(lua_State *state);
    int luaGetObjectPosition(lua_State *state);
    int luaGetStamina(lua_State *state);
    int luaGetHp(lua_State *state);
    int luaSetAttackType(lua_State *state);
    int luaSetAttackDirection(lua_State *state);
    int luaSetBlockDirection(lua_State *state);
    int luaSetMovementType(lua_State *state);
    int luaGetMovementType(lua_State *state);
    int luaGetMe(lua_State *state); //maybe delete
    int luaGetAttackStaminaCost(lua_State *state);
    int luaGetMoveStaminaCost(lua_State *state);
    int luaGetBlockStaminaCost(lua_State *state);
    int luaGetSprintStaminaCost(lua_State *state);
    int luaLevelUp(lua_State *state);
    int luaGetAvailableSkillPoints(lua_State *state);
    int luaGetCurrentExp(lua_State *state);
    int luaGetNextLevelExp(lua_State *state);
    int luaGetParameterLevel(lua_State *state);
    int luaGetParameterLevelUpCost(lua_State *state);

    void luaCountHook(lua_State *state, lua_Debug *ar);

    int luaContinuationTest(lua_State *state, int status, lua_KContext ctx);

    //attributes
    Action action;
    Direction direction;
    AttackType attackType;
    MovementType movementType;
    AttackDirection attackDirection;
    AttackDirection blockDirection;
    const GameObject *target;
    int hitPoints;
    int maxHitPoints;
    int speed;
    int damage;
    int stamina;
    int maxStamina;
    int attackRange;
    long long nextMoveTime;   //probably should refactor. Either unite cooldown variables or move up them.
    long long nextAttackTime; //
    long long nextStaminaRegenTime;
    int moveCooldown;
    int attackCooldown;
    int visionRange;
    bool isStaminaHpRegenAvailable;
    bool isStaminaRegenAvailable;
    int maxStaminaTicksRequirement;
    int maxStaminaTicks;
    int attackStaminaCost;
    int moveStaminaCost;
    int blockStaminaCost; //maybe delete
    int sprintStaminaCost;
    int level;
    int nextLevelExp;
    int currentExp;
    int skillPoints;
    std::unordered_map<Parameters, int> parameters;
    SpriteDirection spriteDirection;
    Inventory inventory;
    lua_State *L;
};
typedef std::shared_ptr<Character> PCharacter;

}
