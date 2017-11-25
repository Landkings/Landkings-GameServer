#pragma once
#include <sstream>
#include <list>
#include <vector>

#include <boost/property_tree/ptree.hpp>

#include "uWS.h"

#include "GameObject.h"

class WsServer
{
public:
    WsServer();
    ~WsServer();
    bool start(uint16_t port);
    void stop();
    std::string error();
    size_t errorCounter();
    bool running();
    bool ready();
    void getObjectsJson(std::vector<Engine::GameObject*> objects, boost::property_tree::ptree& pt);
private:
    uWS::Hub _hub;
    bool _running;
    bool _ready;
    std::list<std::string> _errorList;

    void runHubLoop(uint16_t port);
    // TODO:
    void onConnection();
    void onMessage();
    void onError();
    void onDisconnection();
    void onTransfer();
    void onPing();
};
