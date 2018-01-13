#include "ObjectSpawner.h"

using namespace Engine;

//public methods

// private methods

Vec2i ObjectSpawner::getRandomPosition() {
    return Vec2i(minBoundaries.getX() + (std::rand() % maxBoundaries.getX()),
                 minBoundaries.getY() + (std::rand() % maxBoundaries.getY()));
}
