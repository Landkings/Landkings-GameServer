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

}

bool Engine::SafeZone::inZone(Engine::Character *player) {
    return (player->getPosition() - position).abs() <= zoneTiers[currentZoneTier].radius;
}

Vec2i SafeZone::genNextPosition() {
    int radius = zoneTiers[currentZoneTier].radius;
    return Vec2i( position.getX() + (std::rand() % (2 * radius) - radius),
                  position.getY() + (std::rand() % (2 * radius) - radius));
}
}
