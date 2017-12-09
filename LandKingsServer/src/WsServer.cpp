#include "WsServer.h"

#include <thread>
#include <atomic>
#include <iostream>
#include <functional>
#include <chrono>

#include <boost/property_tree/json_parser.hpp>

#include "Engine.h"
#include "GameObject.h"

using namespace std;
using namespace uWS;
using namespace Engine;
using namespace boost::property_tree;


WsServer::WsServer(Engine::Engine& engine) : _engine(engine), _log("ws-server.log", ios_base::app), _ready(false)
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
    thread([this]()
    {
        while (true) // WsServer custom loop
        {
            _ready = false;
            sendObjects();
            _ready = true;
            while (!_engine.ready)
                continue;
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
    }).detach();
    runHubLoop(port);
    return true;
}

void WsServer::sendObjects()
{
    _engine.waitForMutex(_engine.sceneMutex);
    string objectsMessage = createObjectsMessage();
    _hub.Group<SERVER>::broadcast(objectsMessage.data(), objectsMessage.length(), TEXT);
    _engine.sceneMutex.unlock();
}

string WsServer::createObjectsMessage()
{
    ptree objectsJson;
    ptree playersJson;
    const vector<GameObject*>& objects = _engine.scene.getObjects(); // TODO: block game server thread
    // TODO:
    // players = getPlayers(objects);
    // objects2json(players, playersJson);
    objects2Json(objects, playersJson);
    objectsJson.put<string>("messageType", "objects");
    objectsJson.put_child("players", playersJson);
    stringstream ss;
    json_parser::write_json(ss, objectsJson);
    return ss.str();
}

void WsServer::onConnection(WebSocket<SERVER>* socket, HttpRequest request)
{
    log(string("Connected: ") + socket->getAddress().family + socket->getAddress().address);
    sendMap(socket);
}

void WsServer::sendMap(uWS::WebSocket<SERVER>* socket)
{
    _engine.waitForMutex(_engine.sceneMutex);
    const TileMap& tileMap = _engine.scene.getTileMap();
    ptree result, mapJson;
    int height = tileMap.size(), width = tileMap[0].size();
    result.put<int>("width", width);
    result.put<int>("height", height);
    for (int i = 0; i < height; ++i)
    {
        for (int j = 0; j < width; ++j)
        {
            ptree val;
            val.put("", tileMap[i][j]->getIdx());
            mapJson.push_back(make_pair("", val));
        }
    }
    result.put_child("map", mapJson);
    stringstream ss;
    json_parser::write_json(ss, result);
    socket->send(ss.str().data(), ss.str().length(), TEXT);
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
    MessageType t = getMessageType(incomingJson);
    switch (t)
    {
        case MessageType::sourceCode:
            processPlayerSource(socket, incomingJson);
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

void WsServer::processPlayerSource(uWS::WebSocket<SERVER>* socket, ptree& json)
{
    string nickname, code;
    try
    {
        code = json.get<string>("sourceCode");
        nickname = json.get<string>("nickname");
    }
    catch (exception e)
    {
        log("Invalid source code JSON");
        return;
    }
    log(string("Source code from ") + socket->getAddress().family + socket->getAddress().address +
        " (" + nickname + ") ", true);
    log(code);
    _engine.pushPendingPlayer(nickname, code);
}

WsServer::MessageType WsServer::getMessageType(ptree& message)
{
    string messageTypeString;
    try
    {
        messageTypeString = message.get<string>("messageType");
    }
    catch (exception e)
    {
        return MessageType::unknown;
    }
    if (messageTypeString == "sourceCode")
        return MessageType::sourceCode;
    return MessageType::unknown;
}

void WsServer::objects2Json(const vector<GameObject*>& objects, ptree& pt)
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
