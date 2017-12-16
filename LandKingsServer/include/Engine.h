#pragma once
#include <iostream>
#include <chrono>
#include <unistd.h>
#include <thread>
#include <mutex>
#include <functional>
#include <fstream>
#include <atomic>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

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
    static bool ptreeFromString(const std::string& s, boost::property_tree::ptree& output);
    static void stringFromPtree(const boost::property_tree::ptree& pt, std::string& output);

    enum class MessageType
    {
        unknown, acceptConnection, newPlayer
    };

    MessageType getMessageType(const boost::property_tree::ptree& pt) const;

    void onWsMessage(uWS::WebSocket<uWS::CLIENT>* socket, char* message, size_t length, uWS::OpCode opCode);
    void processMessage(const boost::property_tree::ptree& message);
    void processAcceptConnection(const boost::property_tree::ptree& message);
    void processNewPlayer(const boost::property_tree::ptree& message);
    void processUnknown(const boost::property_tree::ptree& message);

    bool messageServerConnection();

    uWS::Hub* wsHub;
    uWS::WebSocket<uWS::CLIENT>* wsSocket;
    std::atomic<bool> connected;
};

}
