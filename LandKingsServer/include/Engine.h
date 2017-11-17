#pragma once
#include "Scene.h"
#include "GameObject.h"
#include "Position.h"

namespace Engine {

class Engine {
public:
    Engine();
    void run();
private:
    Scene scene;
};

}
