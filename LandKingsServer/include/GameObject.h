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

#undef E

#define E C_ENUM_TO_STRING_HELPER
static std::map<Action, std::string> actionStrings = {
    ACTION
};

static std::map<Direction, std::string> directionStrings = {
    DIRECTION
};

#undef E

const Position directions[4] = {
        Position(0, -1),
        Position(1, 0),
        Position(0, 1),
        Position(-1, 0)
};

class GameObject {
public:
    GameObject(Scene* scene, Position pos = Position(), HitBox hbox = HitBox());
    Position getPosition() const { return position; }
    void setPosition(Position pos) { position = pos; }
    HitBox getHitbox() const { return hbox; }
    void setHitbox(HitBox& hitbox) { hbox = hitbox; }
    std::string getName() { return name; }
    int getX() const { return position.getX(); }
    int getY() const { return position.getY(); }
    int getWidth() const { return hbox.getWidth(); }
    int getHeight() const { return hbox.getHeight(); }
    //std::string getID(); //TODO: make this function

    virtual bool isPassable() { return false; }
    virtual void update() = 0;
protected:
    Position position;
    HitBox hbox;
    Scene *scene;
    std::string name; //?
};

class Character : public GameObject {
public:
    Character(Scene *scene, Position pos = Position(), std::string tmpLuaName = "", HitBox hbox = HitBox(20, 20));
    Character(Scene *scene, std::string luaCode, Position pos = Position());
    void move();
    void attack();
    int write(lua_State *state);
    void update() override;
    int getSpeed() const { return speed; }
    int getHp() const { return hitPoints; }
    void setHp(const int hp) { hitPoints = hp; }
    int getDamage() const { return damage; }
    void setDirection(const Direction dir) { direction = dir; }
    void setTarget (const GameObject *targ) { target = targ; }
    int getSpeedCooldown() const { return speed; }
    long long getNextMoveTime() const { return nextMoveTime; }    
    void setNextMoveTime();
    long long getNextAttackTime() const {return nextAttackTime; }
    //void setNextAttackTime();
    void luaPush(lua_State *state);
    void attack(Character *target);
    void move(Position newPos);
    void takeDamage(int amount);
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
    int luaSetMovementType(lua_State *state);
    int luaGetMovementType(lua_State *state);
    int test(lua_State *stata);
    void init();
    Action action;
    Direction direction;
    AttackType attackType;
    MovementType movementType;
    const GameObject *target;
    int hitPoints;
    int speed;
    int damage;
    int stamina;
    long long nextMoveTime; //probably should refactor. Either unite cooldown variables or move up them.
    long long nextAttackTime;
    int moveCooldown;
    int attackCooldown;
    lua_State *L;
};
typedef std::shared_ptr<Character> PCharacter;

}
