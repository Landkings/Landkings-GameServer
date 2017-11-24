#pragma once
#include "Hitbox.h"

namespace Engine {

class Tile {
public:
    Tile(bool passable) : passable(passable) {}
    bool isPassable() { return passable; }
private:
    bool passable;
};

}
