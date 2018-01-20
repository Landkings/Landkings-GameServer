#pragma once
#include <vector>
#include "Scene.h"
#include "Vec2i.h"
#include "GameObject.h"

namespace Engine {

struct ZoneTier {
public:
    ZoneTier(int stayTime, int shrinkTime, int radius) : stayTime(stayTime), shrinkTime(shrinkTime), radius(radius) {}
    int stayTime, shrinkTime, radius;
};

class SafeZone {
public:
    SafeZone() {}
    SafeZone(Scene* scene, int nextZoneTime, Vec2i spawnPosition, std::vector<ZoneTier> zoneTiers = std::vector<ZoneTier>{
            ZoneTier(1000, 0, 10000),
            ZoneTier(1000, 1000, 100)
        }) :
        scene(scene),
        nextZoneTime(nextZoneTime),
        zoneTiers(zoneTiers),
        lastPosition(spawnPosition),
        position(spawnPosition),
        newPosition(spawnPosition),
        currentZoneTier(0) {}
    void update();
    void luaPush(lua_State *state);
    bool inZone(Character *player);
private:
    Vec2i genNextPosition();
    std::vector<ZoneTier> zoneTiers;
    Scene *scene;
    Vec2i position;
    Vec2i lastPosition;
    Vec2i newPosition;
    int nextZoneTime;
    int currentZoneTier;
};
}
