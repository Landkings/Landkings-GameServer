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
    size_t errorCounter() const;
    bool running() const;

private:
    uWS::Hub _hub;
    bool _running;
    std::list<std::string> _errorList;

    void createObjectsJson(std::vector<Engine::GameObject*> objects, boost::property_tree::ptree& pt);

    void runHubLoop(uint16_t port);
    // TODO:
    void onConnection();
    void onMessage();
    void onError();
    void onDisconnection();
    void onTransfer();
    void onPing();
};
