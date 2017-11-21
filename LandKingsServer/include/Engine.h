#pragma once
#include <iostream>
#include <chrono>
#include <unistd.h>
#include <windows.h>

#include "Scene.h"
#include "GameObject.h"
#include "Position.h"

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
