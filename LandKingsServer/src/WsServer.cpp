#include "WsServer.h"

#include <thread>
#include <atomic>
#include <iostream>
#include <functional>
#include <chrono>
#include <fstream>

#include <boost/property_tree/json_parser.hpp>

#include "Engine.h"
#include "GameObject.h"


using namespace std;
using namespace uWS;
using namespace Engine;
using namespace boost::property_tree;


WsServer::WsServer(Engine::Engine* engine) : _engine(engine)
{

}

WsServer::~WsServer()
{

}

void WsServer::log(string msg)
{
    ofstream logFile("ws-server.log", ios_base::app);
    stringstream resultMsg;
    time_t t = time(nullptr);
    tm* curTime = localtime(&t);
    resultMsg << '('
              << setfill('0') << setw(2) << curTime->tm_mday << 'd' << ' '
              << setfill('0') << setw(2) << curTime->tm_hour << ':'
              << setfill('0') << setw(2) << curTime->tm_min << ':'
              << setfill('0') << setw(2) << curTime->tm_sec
              << ')';
    resultMsg << ' ' << msg << endl;
    cout << resultMsg.str();
    logFile << resultMsg.str();
    logFile.close();
}

bool WsServer::start(uint16_t port)
{
    if (!_hub.listen(port))
    {
        log("Cant listen port: " + to_string(port)); // TODO: info
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
        ptree incoming;
        stringstream jsonStream;
        jsonStream << string(message).substr(0, length);
        string type;
        try
        {
            json_parser::read_json(jsonStream, incoming);
            type = incoming.get<string>("messageType");
        }
        catch (exception e)
        {
            log("Receive invalid JSON: " + jsonStream.str());
            return;
        }
        if (type == "getCharacters")
        {
            ptree objectsJson;
            ptree playersJson;
            vector<GameObject*> objects = _engine->getScene().getObjects();
            // TODO:
            // players = getPlayers(objects);
            // objects2json(players, playersJson);
            objects2Json(objects, playersJson);
            objectsJson.put_child("players", playersJson);
            stringstream ss;
            json_parser::write_json(ss, objectsJson);
            ws->send(ss.str().data(), ss.str().length(), opCode);
        }

    });
    _hub.run();
}

void WsServer::objects2Json(vector<GameObject*>& objects, ptree& pt)
{
    for (int i = 0; i < objects.size(); ++i)
    {
        GameObject* object = objects[i];
        ptree ptObject;
        ptObject.put("x", object->getPosition().getX());
        ptObject.put("y", object->getPosition().getY());
        pt.push_back(make_pair("", ptObject));
        // TODO: pt.put_child(object->getName(), ptObject);
    }
}
