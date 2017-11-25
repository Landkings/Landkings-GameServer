#pragma once
#include <iostream>
#include <chrono>
#include <unistd.h>
#include <thread>

#include "Scene.h"
#include "GameObject.h"
#include "Position.h"

namespace Engine {

class Engine {
public:
    Engine();
    void run();
    Scene* getScene();
private:
    void update();
    Scene scene;
};

}

extern Engine::Engine* enginePtr;
