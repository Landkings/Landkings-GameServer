#include "WsServer.h"

#include <thread>
#include <atomic>
#include <iostream>
#include <functional>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "Engine.h"


using namespace std;
using namespace uWS;
using namespace Engine;
using boost::property_tree::ptree;

WsServer::WsServer() : _running(false)
{

}

WsServer::~WsServer()
{

}

bool WsServer::start(uint16_t port)
{
    if (_running)
    {
        _errorList.push_back("Server still running");
        return false;
    }
    if (!_hub.listen(port))
    {
        _errorList.push_back("Cant listen port: " + to_string(port)); // TODO
        return false;
    }
    runHubLoop(port);
    return _running;
}

void WsServer::runHubLoop(uint16_t port)
{
    cout << "running" << endl;
    // TODO set callbacks
    _hub.onMessage([this](uWS::WebSocket<uWS::SERVER>* ws, char* message, size_t length, uWS::OpCode opCode)
    {
        cout << string(message).substr(0, length) << endl;
        ptree pt;
        stringstream jsonStream;
        jsonStream << string(message).substr(0, length);
        string type = pt.get<string>("messageType");
        if (type == "getCharacters")
        {
            ptree objectsJson;
            vector<GameObject*> characters = enginePtr->getScene()->getObjects();
            getObjectsJson(characters, objectsJson);
            stringstream ss;
            boost::property_tree::json_parser::write_json(ss, pt);
            ws->send(ss.str().data(), ss.str().length(), opCode);
        }
    });
    _running = true;
    _hub.run();
}

void WsServer::getObjectsJson(std::vector<Engine::GameObject*> objects, ptree& pt)
{
    for (int i = 0; i < objects.size(); ++i)
    {
        GameObject* object = objects[i];
        ptree ptObject;
        ptObject.put("x", object->getPosition().getX());
        ptObject.put("y", object->getPosition().getY());
        pt.add_child(object->getName(), ptObject);
    }
}

void WsServer::stop()
{
    _hub.Group<SERVER>::terminate();
    _running = false;
}

bool WsServer::running()
{
    return _running;
}

string WsServer::error()
{
    if (_errorList.empty())
        return string();
    string ret = *(_errorList.begin());
    _errorList.pop_front();
    return ret;
}

size_t WsServer::errorCounter()
{
    return _errorList.size();
}
