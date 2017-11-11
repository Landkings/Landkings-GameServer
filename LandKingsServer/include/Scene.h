#pragma once
#include <vector>
#include <memory>

#include "Constants.h"
#include "Position.h"

namespace Engine {

class GameObject;
typedef std::shared_ptr<GameObject> PGameObject;

class Scene {
public:
    Scene();
    void move(GameObject *object, Position &new_pos);
    void update();
private:
    bool validPosition(Position &pos);
    std::vector<PGameObject> objects;
    int height;
    int width;
};

}
