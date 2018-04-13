#pragma once
#include <vector>
#include "Scene.h"
#include "Vec2i.h"
#include "GameObject.h"
#include "LuaHelper.h"

namespace Engine {

struct ZoneTier {
public:
    ZoneTier(int stayTime, int shrinkTime, int radius) : stayTime(stayTime), shrinkTime(shrinkTime), radius(radius) {}
    int stayTime, shrinkTime, radius;
};

class SafeZone {
public:
    SafeZone() {}
    SafeZone(Scene* scene, std::vector<ZoneTier> zoneTiers = std::vector<ZoneTier>{
            ZoneTier(5000, 1000, 2000),
            ZoneTier(5000, 10000, 500),
            ZoneTier(5000, 5000, 400),
            ZoneTier(3000, 5000, 300),
            ZoneTier(3000, 5000, 200),
            ZoneTier(3000, 5000, 100),
            ZoneTier(3000, 5000, 50),
        });
    void update();
    void luaPush(lua_State *state);
    bool inZone(Character *player);
    Vec2i getPosition() { return position; }
    int getRadius() { return radius; }
private:
    int luaGetPosition(lua_State *state);
    int luaGetRadius(lua_State *state);
    int luaGetNextPosition(lua_State *state);
    Vec2i genNextPosition();
    std::vector<ZoneTier> zoneTiers;
    Scene *scene;
    Vec2i position;
    Vec2i lastPosition;
    Vec2i newPosition;
    int nextZoneTime;
    int currentZoneTier;
    int radius;
};
}
