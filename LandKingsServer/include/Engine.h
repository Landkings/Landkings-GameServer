#pragma once
#include "Scene.h"

namespace Engine {

class Engine {
public:
    Engine();
    void run();
private:
    void update();
    Scene scene;
};

}
