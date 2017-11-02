#pragma once

class HitBox {
public:
    HitBox(int length = 0, int width = 0);
    int getHeight() { return height; }
    int getWidth() { return width; }
private:
    int height;
    int width;
};