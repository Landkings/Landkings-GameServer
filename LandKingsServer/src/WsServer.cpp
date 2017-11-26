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
using namespace boost::property_tree;


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
    return true;
}

void WsServer::runHubLoop(uint16_t port)
{
    // TODO set callbacks
    _hub.onMessage([this](uWS::WebSocket<uWS::SERVER>* ws, char* message, size_t length, uWS::OpCode opCode)
    {
        cout << string(message).substr(0, length) << endl;
        ptree pt;
        stringstream jsonStream;
        jsonStream << string(message).substr(0, length);
        json_parser::read_json(jsonStream, pt);
        string type = pt.get<string>("messageType");
        if (type == "getCharacters")
        {
            ptree objectsJson;
            vector<GameObject*> objects = enginePtr->getScene().getObjects();
            createObjectsJson(objects, objectsJson);
            stringstream ss;
            json_parser::write_json(ss, objectsJson);
            ws->send(ss.str().data(), ss.str().length(), opCode);
        }
    });
    _running = true;
    _hub.run();
}

void WsServer::createObjectsJson(vector<GameObject*> objects, ptree& pt)
{
    for (int i = 0; i < objects.size(); ++i)
    {
        GameObject* object = objects[i];
        ptree ptObject;
        ptObject.put("x", object->getPosition().getX());
        ptObject.put("y", object->getPosition().getY());
        pt.put_child(to_string(i), ptObject);
        // TODO: pt.put_child(object->getName(), ptObject);
    }
}

void WsServer::stop()
{
    _hub.Group<SERVER>::terminate();
    _running = false;
}

bool WsServer::running() const
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

size_t WsServer::errorCounter() const
{
    return _errorList.size();
}
