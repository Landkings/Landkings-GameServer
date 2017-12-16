#pragma once
#include <iostream>
#include <chrono>
#include <unistd.h>
#include <thread>
#include <mutex>
#include <functional>
#include <fstream>
#include <atomic>

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

    Scene scene;

    // ***************
    void onWsMessage(uWS::WebSocket<uWS::CLIENT>* socket, char* message, size_t length, uWS::OpCode opCode);
    bool messageServerConnection();

    uWS::Hub* wsHub;
    uWS::WebSocket<uWS::CLIENT>* wsSocket;
    std::atomic<bool> connected;
};

}
