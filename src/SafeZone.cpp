#include <ctime>
#include "SafeZone.h"

namespace Engine {
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
        if (zoneTiers.size() - 1  == currentZoneTier) {
            currentZoneTier = 0;
            nextZoneTime = scene->getTime() + zoneTiers[0].stayTime;
            radius = zoneTiers[0].radius;
            //lastPosition =
            //position(spawnPosition);
            //newPosition(spawnPosition);
        }
    }
    //std::cout << "Radius: " << radius << std::endl;
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
    lua_pushinteger(state, (int)(zoneTiers[currentZoneTier].radius * ((nextZoneTime - zoneTiers[currentZoneTier].stayTime - scene->getTime()) /
                                                       ((double)zoneTiers[currentZoneTier].shrinkTime))));
    return 1;
}

int SafeZone::luaGetNextPosition(lua_State *state) {
    newPosition.luaPush(state);
    return 1;
}

Vec2i SafeZone::genNextPosition() {
    //int radius = zoneTiers[currentZoneTier].radius;
    //return Vec2i(std::abs((position.getX() + (std::rand() % (2 * radius) - radius))) % scene->getWidth(),
    //             std::abs(position.getY() + (std::rand() % (2 * radius) - radius)) % scene->getHeight());
    std::srand(std::time(0));
    return Vec2i(rand() % (Constants::SCENE_HEIGHT / 10), rand() % (Constants::SCENE_WIDTH / 10));
}
}
