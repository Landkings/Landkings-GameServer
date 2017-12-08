#pragma once
#include <sstream>
#include <list>
#include <vector>
#include <map>
#include <fstream>
#include <mutex>

#include <boost/property_tree/ptree.hpp>

#include "uWS.h"


namespace Engine
{
    class Engine;
    class GameObject;
}

class WsServer
{
public:
    WsServer(Engine::Engine* engine);
    ~WsServer();
    bool start(uint16_t port);
private:
    Engine::Engine* _engine;
    uWS::Hub _hub;
    std::ofstream _log;

    enum class messageType
    {
        unknown, sourceCode
    };

    void log(std::string msg, bool inBuffer = false);
    messageType getMessageType(boost::property_tree::ptree& message);
    void objects2Json(const std::vector<Engine::GameObject*>& objects, boost::property_tree::ptree& pt);
    void runHubLoop(uint16_t port);

    std::string createObjectsMessage();
    void objectsMessageSending();
    void processPlayerSource(uWS::WebSocket<uWS::SERVER>* socket, boost::property_tree::ptree& json);


    void onConnection(uWS::WebSocket<uWS::SERVER>* socket, uWS::HttpRequest request);
    void onDisconnection(uWS::WebSocket<uWS::SERVER>* socket, int code, char* message, size_t length);
    void onMessage(uWS::WebSocket<uWS::SERVER>* socket, char* message, size_t length, uWS::OpCode opCode);
    void onTransfer(uWS::WebSocket<uWS::SERVER>* socket);
    void onPing(uWS::WebSocket<uWS::SERVER>* socket, char* message, size_t length);
    void onError(int err);
};
