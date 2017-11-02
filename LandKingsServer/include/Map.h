#pragma once
#include <vector>
#include "Constants.h"
#include "GameObject.h"

class Map {
public:
    Map();
private:
    std::vector<std::vector<GameObjectPtr>> map;
    int length;
    int width;
};
