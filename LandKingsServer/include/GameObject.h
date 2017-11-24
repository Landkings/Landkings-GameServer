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

/* In the c header file: */
#define E C_ENUM_HELPER
enum Action {
    ACTION
};

enum Direction {
    DIRECTION
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
    GameObject(Scene* scene, Position pos = Position(), std::string tmpLuaName = "", HitBox hbox = HitBox());
    Position getPosition() const { return position; }
    void setPosition(Position pos) { position = pos; }
    HitBox getHitbox() { return hbox; }
    void setHitbox(HitBox& hitbox) { hbox = hitbox; }
    std::string getName() { return name; }
    //std::string getID(); //TODO: make this function

    virtual bool isPassable() { return false; }
    virtual void update() = 0;

    std::string tmpLuaName;
protected:
    Position position;
    HitBox hbox;
    Scene *scene;
    std::string name; //?
};

class Character : public GameObject {
public:
    Character(Scene *scene, Position pos = Position(), std::string tmpLuaName = "", HitBox hbox = HitBox());
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
    void luaPush(lua_State *state);
protected:
    //int move(lua_State *state);
    int setAction(lua_State *state);
    int getAction(lua_State *state);
    int setDirection(lua_State *state);
    int getDirection(lua_State *state);
    int setTarget(lua_State *state);
    int getTarget(lua_State *state);
    int getPosition(lua_State *state);
    int getObjectPosition(lua_State *state);
    int test(lua_State *stata);
    Action action;
    Direction direction;
    const GameObject *target;
    int hitPoints;
    int speed;
    int damage;
    lua_State *L;
};
typedef std::shared_ptr<Character> PCharacter;

}
