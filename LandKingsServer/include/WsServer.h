#pragma once
#include <sstream>
#include <list>
#include <vector>

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

    enum class messageType
    {
        unknown, source, characters,
    };

    void log(std::string msg);
    messageType getMessageType(boost::property_tree::ptree& message);
    void objects2Json(std::vector<Engine::GameObject*>& objects, boost::property_tree::ptree& pt);
    void runHubLoop(uint16_t port);

    void processCharactersQuery(uWS::WebSocket<uWS::SERVER>* socket, uWS::OpCode opCode);

    // TODO:
    void onConnection();
    void onMessage();
    void onError();
    void onDisconnection();
    void onTransfer();
    void onPing();
};
