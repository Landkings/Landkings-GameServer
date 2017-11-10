#pragma once
#include <vector>
#include <memory>

#include "Constants.h"

namespace Engine {

class GameObject;
typedef std::shared_ptr<GameObject> PGameObject;

class Scene {
public:
    Scene();
    void move(GameObject* object);
    void update();
private:

    std::vector<PGameObject> objects;
    int length;
    int width;
};

}
