#pragma once
#include "Hitbox.h"

namespace Engine {

class Tile {
public:
    Tile(bool passable, int idx) : passable(passable), idx(idx) {}
    int getIdx() { return idx; }
    bool isPassable() { return passable; }
private:
    int idx;
    bool passable;
};

}
