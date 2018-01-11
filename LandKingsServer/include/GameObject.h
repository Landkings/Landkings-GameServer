#pragma once
#include <string>
#include <memory>
#include <functional>
#include <map>

#include "Scene.h"
#include "Position.h"
#include "Hitbox.h"
#include "lua.hpp"
#include "LuaHelper.h"

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

const Position directions[4] = {
        Position(0, -1),
        Position(1, 0),
        Position(0, 1),
        Position(-1, 0)
};

class GameObject {
public:
    GameObject(Scene* scene, Position pos = Position(), std::string name = "", HitBox hbox = HitBox());
    Position getPosition() const { return position; }
    virtual void setPosition(Position pos) { position = pos; }
    virtual HitBox getHitbox() const { return hbox; }
    virtual void setHitbox(HitBox& hitbox) { hbox = hitbox; }
    virtual std::string getName() { return name; }
    virtual int getX() const { return position.getX(); }
    virtual int getY() const { return position.getY(); }
    virtual int getWidth() const { return hbox.getWidth(); }
    virtual int getHeight() const { return hbox.getHeight(); }
    virtual std::string getID() const { return name; }
    virtual bool isPassable() { return false; }
    virtual void update() = 0;
protected:
    Position position;
    HitBox hbox;
    Scene *scene;
    std::string name; //?
};

class Item : public GameObject {
public:
    Item(Scene *scene, Position pos, std::string name, HitBox hbox) : GameObject(scene, pos, name, hbox) {}
private:

};

class Character : public GameObject {
public:
    Character(Scene *scene, Position pos = Position(), std::string tmpLuaName = "", HitBox hbox = HitBox(20, 20));
    Character(Scene *scene, std::string luaCode, std::string name, Position pos = Position());
    void move();
    void attack();
    int write(lua_State *state);
    void update() override;
    int getSpeed() const { return speed; }
    int getHp() const { return hitPoints; }
    int getStamina() const { return stamina; }
    int getMaxStamina() const { return maxStamina; }
    int getMaxHp() const { return maxHitPoints; }
    void setHp(const int hp) { hitPoints = hp; }
    int getDamage() const { return damage; }
    void setDirection(const Direction dir) { direction = dir; }
    void setTarget (const GameObject *targ) { target = targ; }
    int getMoveCooldown() const { return speed; }
    long long getNextMoveTime() const { return nextMoveTime; }    
    void setNextMoveTime();
    long long getNextAttackTime() const {return nextAttackTime; }
    int getVisionRange() const { return visionRange; }
    //void setNextAttackTime();
    void luaPush(lua_State *state);
    void attack(Character *target);
    void move(Position newPos);
    void takeDamage(int amount);
    void loadLuaCode(std::string luaCode);
    bool isBlocking() const { return action == Action::Block; }
    AttackDirection getBlockDirection() const { return blockDirection; }
    AttackDirection getAttackDirection() const { return attackDirection; }
    void loseStamina(int amount);
    void gainStamina(int amount);
    void gainDefaultStamina();
    long long getNextStaminaRegenTime() const { return nextStaminaRegenTime; }
    void gainHp(int amount);
    bool isOnCooldown() const { return nextAttackTime > scene->getTime() || nextMoveTime > scene->getTime(); }
    int getAttackRange() const { return attackRange; }
    //bool getStaminaHpRegenAvailable() const { return isStaminaHpRegenAvailable; }
    //void tryToRegenDefaultStamina();
    SpriteDirection getSpriteDirection() const { return spriteDirection; }
    ~Character();
    std::string tmpLuaName;
protected:
    //int move(lua_State *state);
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
    int luaGetMe(lua_State *state);
    int test(lua_State *stata);
    void init();
    void closeLuaState();
    void initLuaState();
    void block();
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
    long long nextMoveTime; //probably should refactor. Either unite cooldown variables or move up them.
    long long nextAttackTime;
    long long nextStaminaRegenTime;
    int moveCooldown;
    int attackCooldown;
    int visionRange;
    bool isStaminaHpRegenAvailable;
    int maxStaminaTicksRequirement;
    int maxStaminaTicks;
    SpriteDirection spriteDirection;
    lua_State *L;
};
typedef std::shared_ptr<Character> PCharacter;

}
