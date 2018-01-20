#include "SafeZone.h"

namespace Engine {
void Engine::SafeZone::update() {
    if (nextZoneTime == scene->getTime()) {
        if (currentZoneTier < zoneTiers.size() - 1) {
            nextZoneTime += zoneTiers[currentZoneTier].shrinkTime + zoneTiers[currentZoneTier + 1].stayTime; //+1?
            ++currentZoneTier;
            lastPosition = newPosition;
            newPosition = genNextPosition();
            update(); //TODO: verify
        }
    }
    else {
        if(currentZoneTier > 0) {
            if (scene->getTime() < nextZoneTime - zoneTiers[currentZoneTier].stayTime) {
                position = lastPosition  + (lastPosition - newPosition) * ((nextZoneTime - zoneTiers[currentZoneTier].stayTime - scene->getTime()) /
                        ((double)zoneTiers[currentZoneTier].shrinkTime));
            }
        }
    }
}

void Engine::SafeZone::luaPush(lua_State *state) {
    SafeZone **Pzone = (SafeZone**)lua_newuserdata(state, sizeof(SafeZone*));
    *Pzone = this;
    if (luaL_newmetatable(state, "SafeZoneMetaTable")) {
        lua_pushvalue(state, -1);
        lua_setfield(state, -2, "__index");

        luaL_Reg SafeZoneMethods[] = {
            "getPosition", &dispatch<SafeZone, &SafeZone::luaGetPosition>,
            "getRadius", &dispatch<SafeZone, &SafeZone::luaGetRadius>,
            nullptr, nullptr
        };
        luaL_setfuncs(state, SafeZoneMethods, 0);
    }
    lua_setmetatable(state, -2);
}

bool Engine::SafeZone::inZone(Engine::Character *player) {
    return (player->getPosition() - position).abs() <= zoneTiers[currentZoneTier].radius;
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

Vec2i SafeZone::genNextPosition() {
    int radius = zoneTiers[currentZoneTier].radius;
    return Vec2i( (position.getX() + (std::rand() % (2 * radius) - radius)) % scene->getWidth(),
                  position.getY() + (std::rand() % (2 * radius) - radius) % scene->getHeight());
}
}
