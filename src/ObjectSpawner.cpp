#include "ObjectSpawner.h"

using namespace Engine;

//public methods

// private methods

Vec2i ObjectSpawner::getRandomPosition() {
    int offset = 40;
    std::srand(time(0));
    return Vec2i(minBoundaries.getX() + offset + (std::rand() % (maxBoundaries.getX() - offset * 2)),
                 minBoundaries.getY() + offset + (std::rand() % (maxBoundaries.getY() - offset * 2)));
}
