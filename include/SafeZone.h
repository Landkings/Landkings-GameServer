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
            ZoneTier(10000, 0, 10000),
            ZoneTier(10000, 10000, 100)
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
