#pragma once
#include <iostream>
#include <chrono>
#include <unistd.h>
#include <thread>
#include <mutex>
#include <functional>
#include <fstream>
#include <atomic>
#include <string>

#include "document.h"

#include <uWS/uWS.h>

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

    enum class InputMessageType
    {
        unknown = -1, acceptConnection = 'c', newPlayer = 'p'
    };

    enum class OutputMessageType
    {
        unknown = -1, loadMap = 'm', loadObjects = 'o'
    };

    InputMessageType getMessageType(char firstChar) const;

    void onMsMessage(uWS::WebSocket<uWS::CLIENT>* socket, char* message, size_t length, uWS::OpCode opCode);
    void processAcceptConnection();
    void processNewPlayer(const char* message, size_t length);

    bool messageServerConnection();
    void messageServerThreadFunction();
    void runTimer(std::atomic<bool>& stopFlag, std::atomic<bool>& overFlag, unsigned seconds);
    void runConnection();
    void setMessageType(OutputMessageType type, rapidjson::StringBuffer& buffer);

    uWS::Hub* msHub;
    uWS::WebSocket<uWS::CLIENT>* msSocket;
    std::atomic<bool> connected;
    std::atomic<bool> gameServerKnow;
    std::atomic<bool> messageServerKnow;
    std::atomic<bool> msThreadTerminated;
};

}
