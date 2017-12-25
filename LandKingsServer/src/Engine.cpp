#include "Engine.h"

static constexpr uint16_t defaultWsServerPort = 19998;

Engine::Engine::Engine() : wsSocket(nullptr) {
    connected.store(false);
}

void Engine::Engine::run() {
    //scene.addObject(new Character(&scene, Position(20, 20), "p1.lua"));
    auto previous = std::chrono::system_clock::now();
    auto lag = previous - previous;
    int cnt = 0;
    int ticks = 0;
    if (!messageServerConnection()) {
        std::cout << "Timeout for connection" << std::endl;
        return;
    }

    while (true) {
        //auto current = std::chrono::system_clock::now();
        //auto elapsed = current - previous;
        //previous = xcurrent;
        //lag += elapsed;

        //while (lag >= 30) {
            update();
        //    lag -= 30;
        //}

        //scene.print();
        ++ticks;
        std::string objectsJSON = scene.getObjectsJSON();
        if (connected.load()) {
            if (ticks > 32) {
                wsSocket->send(objectsJSON.c_str());
                ticks = 0;
            }
        }
        else {
            delete wsHub;
            if (messageServerConnection())
                continue;
            std::cout << "Timeout for connection" << std::endl;
            return;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void Engine::Engine::update() {
    scene.update();
}


// **********

using namespace std;
using namespace uWS;
using namespace boost::property_tree;


bool Engine::Engine::messageServerConnection()
{
    std::atomic<bool> timeout, stopTimer;
    timeout = false; stopTimer = false;
    std::cout << "Try connect to message server" << std::endl;
    std::thread([this, &timeout, &stopTimer]()
    {
        int secondsCounter = 0;
        while (!connected.load() && !stopTimer.load())
        {
            std::cout << 10 - secondsCounter << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
            if (++secondsCounter == 10)
            {
                timeout.store(true);
                return;
            }
        }
    }).detach();
    while (true)
    {
        std::atomic<bool> threadTerminated;
        threadTerminated = false;
        std::thread([this, &threadTerminated]()
        {
            auto messageHandler = std::bind(&Engine::Engine::onWsMessage, this,
                                            std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
            wsHub = new uWS::Hub();
            wsHub->onMessage(messageHandler);
            std::map<std::string, std::string> header;
            std::getline(std::ifstream("secret.txt"), header["secret"]);
            wsHub->connect("ws://localhost:19998", nullptr, header);
            wsHub->run();
            connected.store(false);
            threadTerminated.store(true);
        }).detach();
        while (!threadTerminated.load() && !connected.load() && !timeout.load())
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        if (connected.load())
        {
            stopTimer.store(true);
            return true;
        }
        if (timeout.load())
            return false;
    }
}

void Engine::Engine::onWsMessage(uWS::WebSocket<uWS::CLIENT>* socket, char* message, size_t length, uWS::OpCode opCode)
{
    std::cout << "Message: " << std::string(message).substr(0, length) << std::endl;
    ptree pt;
    if (!ptreeFromString(std::string(message, length), pt))
    {
        // something
        return;
    }
    wsSocket = socket;
    processMessage(pt);
}

void Engine::Engine::processMessage(const ptree& message)
{
    MessageType t = getMessageType(message);
    switch (t)
    {
        case MessageType::acceptConnection:
            processAcceptConnection(message);
            return;
        case MessageType::newPlayer:
            processNewPlayer(message);
            return;
        case MessageType::unknown:
            processUnknown(message);
            return;
    }
}

void Engine::Engine::processAcceptConnection(const ptree& message)
{
    connected.store(true);
    std::string tileMapJSON = scene.getTileMapJSON();
    wsSocket->send(tileMapJSON.c_str());
}

void Engine::Engine::processNewPlayer(const ptree& message)
{
    string src, nick;
    try
    {
        src = message.get<string>("sourceCode");
        nick = message.get<string>("nickname");
    }
    catch (...)
    {
        // TODO: something
    }
    scene.addPlayer(nick, src);
}

void Engine::Engine::processUnknown(const ptree& message)
{

}

Engine::Engine::MessageType Engine::Engine::getMessageType(const ptree& message) const
{
    std::string messageType;
    try
    {
        messageType = message.get<std::string>("messageType");
    }
    catch (...)
    {
        return MessageType::unknown;
    }
    if (messageType == "acceptConnection")
        return MessageType::acceptConnection;
    if (messageType == "newPlayer")
        return MessageType::newPlayer;
    return MessageType::unknown;
}

bool Engine::Engine::ptreeFromString(const std::string& s, ptree& output)
{
    stringstream ss;
    ss << s;
    try
    {
        json_parser::read_json(ss, output);
    }
    catch (...)
    {
        return false;
    }
    return true;
}

void Engine::Engine::stringFromPtree(const ptree& pt, std::string& output)
{
    stringstream ss;
    json_parser::write_json(ss, pt);
    output = ss.str();
}

// **********
