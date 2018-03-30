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
    SafeZone(Scene* scene, Vec2i spawnPosition, std::vector<ZoneTier> zoneTiers = std::vector<ZoneTier>{
            ZoneTier(100, 0, 10000),
            ZoneTier(100, 1000, 9000),
            ZoneTier(100, 1000, 8000),
            ZoneTier(1000, 1000, 6000),
            ZoneTier(1000, 1000, 4000),
            ZoneTier(1000, 1000, 1000),
            ZoneTier(10000, 10000, 500),
            ZoneTier(10000, 10000, 100),
            ZoneTier(100, 1000, 0),
            ZoneTier(100, 100, 0),
        }) :
        scene(scene),
        nextZoneTime(scene->getTime() + zoneTiers[0].stayTime),
        radius(zoneTiers[0].radius),
        zoneTiers(zoneTiers),
        lastPosition(spawnPosition),
        position(spawnPosition),
        newPosition(spawnPosition),
        currentZoneTier(0) {}
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
