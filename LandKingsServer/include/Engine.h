#pragma once
#include <iostream>
#include <chrono>
#include <unistd.h>
#include <thread>
#include <mutex>
#include <functional>
#include <fstream>

#include "uWS.h"

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

    void waitForMutex(std::mutex& m, const std::chrono::microseconds& interval = std::chrono::microseconds(10));

    Scene scene;

    // ***************
    void onWsMessage(uWS::WebSocket<uWS::CLIENT>* socket, char* message, size_t length, uWS::OpCode opCode);

    uWS::Hub wsHub;
    uWS::WebSocket<uWS::CLIENT>* wsSocket;
};

}
