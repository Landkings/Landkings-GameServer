#pragma once
#include <iostream>
#include <chrono>
#include <unistd.h>
#include <thread>
#include <mutex>

#include "Scene.h"
#include "GameObject.h"
#include "Position.h"
#include "WsServer.h"

namespace Engine {


class Engine {
    friend class ::WsServer;
public:
    Engine();
    void run();
private:
    void update();

    void waitForMutex(std::mutex&m, const std::chrono::microseconds& interval = std::chrono::microseconds(10));
    void pushPendingPlayer(std::string& nickname, std::string& sourceCode);
    void addPendingPlayers();

    WsServer wsServer;
    Scene scene;
    std::mutex sceneMutex;
    std::vector<std::pair<std::string, std::string>> pendingPlayers;
    std::mutex pendingPlayersMutex;
    bool ready;
};

}
