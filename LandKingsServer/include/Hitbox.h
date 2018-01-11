#pragma once

#include "Vec2i.h"

namespace Engine {

class HitBox {
public:
    HitBox(int length = 0, int width = 0);
    int getHeight() const { return height; }
    int getWidth() const { return width; }
    bool isCollide(const HitBox& target);
    bool isCollide(const Vec2i &lb, const Vec2i &ru);
private:
    int height;
    int width;
};

}
