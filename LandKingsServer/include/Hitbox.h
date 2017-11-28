#pragma once

#include "Position.h"

namespace Engine {

class HitBox {
public:
    HitBox(int length = 0, int width = 0);
    int getHeight() const { return height; }
    int getWidth() const { return width; }
    bool isCollide(const HitBox& target);
    bool isCollide(const Position &lb, const Position &ru);
private:
    int height;
    int width;
};

}
