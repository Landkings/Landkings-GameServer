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


WsServer::WsServer(Engine::Engine* engine) : _engine(engine), _log("ws-server.log", ios_base::app)
{

}

WsServer::~WsServer()
{
    _hub.Group<SERVER>::close(1000);
    _log.close();
}

void WsServer::log(string msg, bool inBuffer)
{
    static stringstream buffer;
    if (buffer.str().length() == 0)
    {
        time_t t = time(nullptr);
        tm* curTime = localtime(&t);
        buffer << '('
                  << setfill('0') << setw(2) << curTime->tm_mday << 'd' << ' '
                  << setfill('0') << setw(2) << curTime->tm_hour << ':'
                  << setfill('0') << setw(2) << curTime->tm_min << ':'
                  << setfill('0') << setw(2) << curTime->tm_sec
                  << ')';
        buffer << ' ' << msg << '\n';
    }
    else
        buffer << msg << '\n';
    if (!inBuffer)
    {
        cout << buffer.str();
        _log << buffer.str();
        _log.flush();
        buffer.str(string());
        buffer.clear();
    }
}

bool WsServer::start(uint16_t port)
{
    if (!_hub.listen(port))
    {
        log("Cant listen port: " + to_string(port));
        return false;
    }
    runHubLoop(port);
    return true;
}

void WsServer::onConnection(WebSocket<SERVER>* socket, HttpRequest request)
{
    log(string("Connected: ") + socket->getAddress().family + socket->getAddress().address);
}

void WsServer::onDisconnection(uWS::WebSocket<SERVER>* socket, int code, char* message, size_t length)
{
    log(string("Disconnected: ") + socket->getAddress().family + socket->getAddress().address, true);
    log("With code: " + to_string(code));
}

void WsServer::onMessage(uWS::WebSocket<SERVER>* socket, char* message, size_t length, OpCode opCode)
{
    cout << string(message).substr(0, length) << endl;
    ptree incomingJson;
    stringstream incJsonStream;
    incJsonStream << string(message).substr(0, length);
    try
    {
        json_parser::read_json(incJsonStream, incomingJson);
    }
    catch (exception e)
    {
        log("Receive invalid JSON: " + incJsonStream.str());
        return;
    }
    messageType t = getMessageType(incomingJson);
    switch (t)
    {
        case messageType::characters:
            processCharactersQuery(socket);
            return;
        case messageType::source:
            processSourceExecution(socket, incomingJson);
            return;
        default:
            socket->send("Not implemented");
    }
}

void WsServer::onTransfer(uWS::WebSocket<SERVER>* socket)
{

}

void WsServer::onPing(uWS::WebSocket<SERVER>* socket, char* message, size_t length)
{
    log(string("Ping from ") + socket->getAddress().family + socket->getAddress().address);
    socket->send("pong", 4, OpCode::PONG);
}

void WsServer::onError(int err)
{
    log("Error: " + to_string(err));
}

void WsServer::runHubLoop(uint16_t port)
{
     auto connectionHandler = bind(&WsServer::onConnection, this,
                                   placeholders::_1, placeholders::_2);
     auto disconnectionHandler = bind(&WsServer::onDisconnection, this,
                                      placeholders::_1, placeholders::_2, placeholders::_3, placeholders::_4);
     auto messageHandler = bind(&WsServer::onMessage, this,
                                 placeholders::_1, placeholders::_2, placeholders::_3, placeholders::_4);
     auto transferHandler = bind(&WsServer::onTransfer, this,
                                 placeholders::_1);
     auto pingHandler = bind(&WsServer::onPing, this,
                             placeholders::_1, placeholders::_2, placeholders::_3);
     auto errorHandler = bind(&WsServer::onError, this,
                                      placeholders::_1);
    _hub.onConnection(connectionHandler);
    _hub.onDisconnection(disconnectionHandler);
    _hub.onMessage(messageHandler);
    _hub.onTransfer(transferHandler);
    _hub.onPing(pingHandler);
    _hub.onError(errorHandler);
    _hub.run();
}

void WsServer::processCharactersQuery(uWS::WebSocket<SERVER>* socket)
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
    socket->send(ss.str().data(), ss.str().length(), uWS::TEXT);
}

void WsServer::processSourceExecution(uWS::WebSocket<SERVER>* socket, ptree& json)
{
    /* TODO: Engine::tryCompileCode(socket)
     * {
     *      if (!compiled)
     *          socket->send("compile error")
     * }
    */
}

WsServer::messageType WsServer::getMessageType(ptree& message)
{
    string messageTypeString;
    try
    {
        messageTypeString = message.get<string>("messageType");
    }
    catch (exception e)
    {
        return messageType::unknown;
    }
    if (messageTypeString == "getCharacters")
        return messageType::characters;
    return messageType::unknown;
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
