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

    enum class MessageType
    {
        unknown, acceptConnection, newPlayer
    };

    MessageType getMessageType(const rapidjson::Document& message) const;

    void onWsMessage(uWS::WebSocket<uWS::CLIENT>* socket, char* message, size_t length, uWS::OpCode opCode);
    void processMessage(const rapidjson::Document& message);
    void processAcceptConnection(const rapidjson::Document& message);
    void processNewPlayer(const rapidjson::Document& message);
    void processUnknown(const rapidjson::Document& message);

    bool messageServerConnection();

    uWS::Hub* wsHub;
    uWS::WebSocket<uWS::CLIENT>* wsSocket;
    std::atomic<bool> connected;
};

}
