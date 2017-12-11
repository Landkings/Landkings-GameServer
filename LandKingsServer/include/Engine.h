#pragma once
#include <iostream>
#include <chrono>
#include <unistd.h>
#include <thread>
#include <mutex>

#include "Scene.h"
#include "GameObject.h"
#include "Position.h"
#include "uWS.h"

namespace Engine {


class Engine {
public:
    Engine();
    void run();
private:
    void update();

    void waitForMutex(std::mutex& m, const std::chrono::microseconds& interval = std::chrono::microseconds(10));

    Scene scene;
};

}
