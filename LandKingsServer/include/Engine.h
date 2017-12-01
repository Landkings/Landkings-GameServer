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
    const Scene& getScene() const;
private:
    void startServer();
    void update();
    Scene scene;
};

}
