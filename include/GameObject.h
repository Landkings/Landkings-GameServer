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
  E(Move, Move) \
  E(Attack, Attack) \
  E(Block, Block) \
  E(Empty, Empty) \

#define DIRECTION \
  E(Up, Up) \
  E(Right, Right) \
  E(Down, Down) \
  E(Left, Left) \
  E(Unknown, Unknown) \

#define ATTACK_TYPE \
  E(Fast, Fast) \
  E(Strong, Strong) \

#define ATTACK_DIRECTION \
  E(Head, Head) \
  E(Torso, Torso) \
  E(Legs, Legs) \

#define MOVEMENT_TYPE \
  E(Default, Default) \
  E(Sprint, Sprint) \

#define E C_ENUM_HELPER
enum Action {
    ACTION
};

enum Direction {
    DIRECTION
};

enum AttackType {
    ATTACK_TYPE
};

enum MovementType {
    MOVEMENT_TYPE
};

enum AttackDirection {
    ATTACK_DIRECTION
};
#undef E

#define E C_ENUM_TO_STRING_HELPER
static std::map<Action, std::string> actionStrings = {
    ACTION
};

static std::map<Direction, std::string> directionStrings = {
    DIRECTION
};
#undef E

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
protected:
    Vec2i position;
    HitBox hbox;
    Scene *scene;
    std::string name; //?
};

class Item : public GameObject {
public:
    Item(Scene *scene, Vec2i pos, std::string name, HitBox hbox, int size) :
        GameObject(scene, pos, name, hbox),
        isUsed(false),
        size(size),
        isInInventory(false) {
    }
    virtual void use(Character* target) = 0;
    virtual bool used() const { return isUsed; }
    virtual int getSize() const { return size; }
    //virtual void collect(Character* target);
    virtual bool inInventory() const { return isInInventory; }
    virtual void luaPush(lua_State *state) = 0;
protected:
    bool isUsed;
    int size;
    bool isInInventory;
};

class HealingItem : public Item {
public:
    HealingItem(Scene *scene, Vec2i pos, HitBox hbox, int amount, int size) :
        Item(scene, pos, "HealingItem", hbox, size),
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
    void luaPush(lua_State *state);
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
    GameObject* clone();
    ~Character();

    //getters and setters
    int getSpeed() const { return speed; }
    int getHp() const { return hitPoints; }
    void setHp(const int hp) { hitPoints = hp; }
    int getMaxHp() const { return maxHitPoints; }
    int getStamina() const { return stamina; }
    int getMaxStamina() const { return maxStamina; }
    int getDamage() const { return damage; }
    void setDirection(const Direction dir) { direction = dir; }
    void setTarget (const GameObject *targ) { target = targ; }
    int getMoveCooldown() const { return moveCooldown; }
    long long getNextMoveTime() const { return nextMoveTime; }    
    long long getNextAttackTime() const {return nextAttackTime; }
    int getVisionRange() const { return visionRange; }
    SpriteDirection getSpriteDirection() const { return spriteDirection; }
    bool isOnCooldown() const { return nextAttackTime > scene->getTime() || nextMoveTime > scene->getTime(); }
    int getAttackRange() const { return attackRange; }
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
    int maxStaminaTicksRequirement;
    int maxStaminaTicks;
    int attackStaminaCost;
    int moveStaminaCost;
    int blockStaminaCost; //maybe delete
    int sprintStaminaCost;
    SpriteDirection spriteDirection;
    Inventory inventory;
    lua_State *L;
};
typedef std::shared_ptr<Character> PCharacter;

}
