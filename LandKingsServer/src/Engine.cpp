#include "Engine.h"

#include "stringbuffer.h"

using namespace rapidjson;

static constexpr uint16_t defaultMsPort = 19998;
static constexpr unsigned defaultReconnectionTime = 25; // s

Engine::Engine::Engine() {

}

void Engine::Engine::run() {
    //scene.addObject(new Character(&scene, Position(20, 20), "p1.lua"));
    auto previous = std::chrono::system_clock::now();
    auto lag = previous - previous;
    int cnt = 0;
    int ticks = 0;
    connected = false;
    gameServerKnow = false;
    messageServerKnow = false;
    if (!messageServerConnection())
        return;

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
        if (!connected.load())
        {
            gameServerKnow.store(true);
            while (!messageServerKnow.load())
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            messageServerKnow.store(false);
            while (!msThreadTerminated.load())
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            if (messageServerConnection())
                continue;
            std::cout << "Timeout for connection" << std::endl;
            return;
        }
        if (++ticks > 32)
        {
            StringBuffer buffer;
            setMessageType(OutputMessageType::loadObjects, buffer);
            scene.createObjectsMessage(buffer);
            msSocket->send(buffer.GetString(), buffer.GetLength(), uWS::TEXT);
            ticks = 0;
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

void Engine::Engine::runTimer(std::atomic<bool>& stopFlag, std::atomic<bool>& overFlag, unsigned seconds)
{
    stopFlag = false; overFlag = false;
    std::thread([&stopFlag, &overFlag](unsigned seconds)
    {
        unsigned secondsCounter = 0;
        while (true)
        {
            std::cout << seconds - secondsCounter << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
            if (stopFlag.load())
                return;
            if (++secondsCounter == seconds)
            {
                overFlag.store(true);
                return;
            }
        }
    }, seconds).detach();
}

void Engine::Engine::runConnection()
{
    std::thread([this]()
    {
        msThreadTerminated.store(false);
        auto messageHandler = std::bind(&Engine::Engine::onMsMessage, this,
                                        std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
        msHub = new uWS::Hub();
        msHub->onMessage(messageHandler);
        std::map<std::string, std::string> header;
        std::getline(std::ifstream("secret.txt"), header["secret"]);
        msHub->connect("ws://localhost:19998", nullptr, header, defaultReconnectionTime);
        msHub->run();
        // connection lost
        if (connected.load()) // not first time connection
        {
            connected.store(false);
            messageServerKnow.store(true);
            while (!gameServerKnow.load())
                this_thread::sleep_for(chrono::milliseconds(40));
            gameServerKnow.store(false);
        }
        msHub->getDefaultGroup<CLIENT>().terminate();
        delete msHub;
        msThreadTerminated.store(true);
    }).detach();
}

bool Engine::Engine::messageServerConnection()
{
    std::cout << "Try connect to message server" << std::endl;
    std::atomic<bool> timeout;
    runTimer(connected, timeout, defaultReconnectionTime);
    while (true)
    {
        runConnection();
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
        if (connected.load())
            return true;
        if (timeout.load())
        {
            do
            {
                if (connected.load())
                    return true;
                if (msThreadTerminated.load())
                    return false;
                std::this_thread::sleep_for(std::chrono::milliseconds(250));
            } while (!msThreadTerminated.load() && !connected.load());
        }
        while (!msThreadTerminated.load())
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

void Engine::Engine::onMsMessage(uWS::WebSocket<uWS::CLIENT>* socket, char* message, size_t length, uWS::OpCode opCode)
{
    InputMessageType type = getMessageType(*message);
    switch (type)
    {
        case InputMessageType::acceptConnection:
            msSocket = socket;
            processAcceptConnection();
            return;
        case InputMessageType::newPlayer:
            processNewPlayer(message + 1, length - 1);
            return;
        default:
            cout << "Invalid message type" << endl;
            return;
    }
}

void Engine::Engine::processAcceptConnection()
{
    connected.store(true);
    StringBuffer buffer;
    setMessageType(OutputMessageType::loadMap, buffer);
    scene.createMapMessage(buffer);
    msSocket->send(buffer.GetString(), buffer.GetLength(), TEXT);
}

void Engine::Engine::processNewPlayer(const char* message, size_t length)
{
    string m(message, length);
    int idx = m.find_first_of('\n');
    scene.addPlayer(m.substr(0, idx), m.substr(idx + 1));
}

Engine::Engine::InputMessageType Engine::Engine::getMessageType(char firstChar) const
{
    switch (firstChar)
    {
        case 'c':
            return InputMessageType::acceptConnection;
        case 'p':
            return InputMessageType::newPlayer;
        default:
            return InputMessageType::unknown;
    }
}

void Engine::Engine::setMessageType(OutputMessageType type, StringBuffer& buffer)
{
    buffer.Put(static_cast<char>(type));
}
