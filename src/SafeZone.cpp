#include <ctime>
#include "SafeZone.h"

namespace Engine {

SafeZone::SafeZone(Scene* scene, std::vector<ZoneTier> zoneTiers) :
    scene(scene),
    nextZoneTime(scene->getTime() + zoneTiers[0].stayTime),
    radius(zoneTiers[0].radius),
    zoneTiers(zoneTiers),
    currentZoneTier(0) {
    srand(unsigned(std::time(0)));
    int x, y;
    if (scene->getWidth() <= radius * 2 || scene->getHeight() <= radius * 2) {
        x = scene->getWidth() / 2;
        y = scene->getHeight() / 2;
    } else {
        x = rand() % (scene->getWidth() - radius * 2) + radius;
        y = rand() % (scene->getHeight() - radius * 2) + radius;
    }
    Vec2i spawnPosition(x, y);
    lastPosition = spawnPosition;
    position = spawnPosition;
    newPosition = spawnPosition;
}


void SafeZone::update() {
    if (nextZoneTime == scene->getTime()) {
        if (currentZoneTier < zoneTiers.size() - 1) {
            nextZoneTime += zoneTiers[currentZoneTier + 1].shrinkTime + zoneTiers[currentZoneTier + 1].stayTime; //+1?
            radius = zoneTiers[currentZoneTier].radius;
            ++currentZoneTier;
            lastPosition = newPosition;
            position = lastPosition;
            newPosition = genNextPosition();
        }
    }
    else {
        if(currentZoneTier > 0) {
            if (scene->getTime() < nextZoneTime - zoneTiers[currentZoneTier].stayTime) {
                double progress = 1.0 - ((nextZoneTime - zoneTiers[currentZoneTier].stayTime - scene->getTime()) / ((double)zoneTiers[currentZoneTier].shrinkTime));
                position = lastPosition  - (lastPosition - newPosition) * progress;
                radius = zoneTiers[currentZoneTier - 1].radius - progress * (zoneTiers[currentZoneTier - 1].radius - zoneTiers[currentZoneTier].radius);
            }
        }
    }
}

void SafeZone::luaPush(lua_State *state) {
    SafeZone **Pzone = (SafeZone**)lua_newuserdata(state, sizeof(SafeZone*));
    *Pzone = this;
    if (luaL_newmetatable(state, "SafeZoneMetaTable")) {
        lua_pushvalue(state, -1);
        lua_setfield(state, -2, "__index");

        luaL_Reg SafeZoneMethods[] = {
            "getPosition", dispatch<SafeZone, &SafeZone::luaGetPosition>,
            "getRadius", dispatch<SafeZone, &SafeZone::luaGetRadius>,
            "getNextPosition", dispatch<SafeZone, &SafeZone::luaGetNextPosition>,
            nullptr, nullptr
        };
        luaL_setfuncs(state, SafeZoneMethods, 0);
    }
    lua_setmetatable(state, -2);
}

bool SafeZone::inZone(Engine::Character *player) {
    return (player->getPosition() - position).abs() <= radius;
}

int SafeZone::luaGetPosition(lua_State *state) {
    position.luaPush(state);
    return 1;
}

int SafeZone::luaGetRadius(lua_State *state) {
//    lua_pushinteger(state, (int)(zoneTiers[currentZoneTier].radius * ((nextZoneTime - zoneTiers[currentZoneTier].stayTime - scene->getTime()) /
//                                                                      ((double)zoneTiers[currentZoneTier].shrinkTime))));
    lua_pushinteger(state, radius);
    return 1;
}

int SafeZone::luaGetNextPosition(lua_State *state) {
    newPosition.luaPush(state);
    return 1;
}

Vec2i SafeZone::genNextPosition() {
    if (scene->getWidth() <= radius * 2 || scene->getHeight() <= radius * 2)
        return Vec2i(scene->getWidth() / 2, scene->getHeight() / 2);
    int r = radius - zoneTiers[currentZoneTier].radius;
    std::srand(std::time(0));
    int x = rand() % (r * 2) - r;
    int range = pow(std::abs(pow(r, 2) + pow(x, 2)), 0.5);
    int y = rand() % (range * 2) - range;
    x += position.getX();
    y += position.getY();
    if (x > scene->getWidth())
        x = scene->getWidth() * 2 - x;
    if (y > scene->getHeight())
        y = scene->getHeight() * 2 - y;
    return Vec2i(x, y);

//    std::srand(std::time(0));
//    int radius = zoneTiers[currentZoneTier].radius;
//    return Vec2i(std::abs((position.getX() + (std::rand() % (2 * radius) - radius))) % scene->getWidth(),
//                 std::abs(position.getY() + (std::rand() % (2 * radius) - radius)) % scene->getHeight());
//    std::srand(std::time(0));
//    return Vec2i(rand() % (Constants::SCENE_HEIGHT / 10), rand() % (Constants::SCENE_WIDTH / 10));
}
}
