#include "Engine.h"

#include <iomanip>

#include "stringbuffer.h"

using namespace rapidjson;
using namespace uWS;
using namespace std;


static constexpr uint16_t defaultMsPort = 19998;
static constexpr unsigned defaultReconnectionTime = 25;
static constexpr unsigned defaultLogInterval = 1000;

bool Engine::Engine::expectedFalse = false;


Engine::Engine::Engine() {

}

// *** START ***

void Engine::Engine::run() {
    startPoint = chrono::time_point_cast<chrono::seconds>(chrono::system_clock::now());
    init();
    if (connected.load())
        mainLoop();
    beforeExit();
    return;
}

void Engine::Engine::init()
{
    terminationSignal = false;
    logCaptured = false;
    logTermSignal = false;
    logThreadTerminated = false;
    connected = false;
    gameServerKnow = false;
    messageServerKnow = false;
    runLog();
    customSleep<milli>(100);
    if (!messageServerConnection())
        log("Can't connect to message server");
}

void Engine::Engine::mainLoop()
{
    //scene.addObject(new Character(&scene, Position(20, 20), "p1.lua"));
    auto previous = std::chrono::system_clock::now();
    auto lag = previous - previous;
    int cnt = 0;
    int ticks = 0;

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
            log("Connection with MS lost");
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

void Engine::Engine::beforeExit()
{
    if (connected.load())
        msHub->Group<CLIENT>::close();
    logTermSignal.store(true);
    while (!logThreadTerminated.load())
        customSleep<milli>(10);
}

void Engine::Engine::terminate()
{
    terminationSignal.store(true);
    beforeExit();
}

// *** FUNCTIONS ***

void Engine::Engine::update() {
    scene.update();
}

void Engine::Engine::runLog() {
    logStream.open("game-server.log", std::ios_base::app);
    thread([this]()
    {
        while (true)
        {
            customSleep<milli>(defaultLogInterval);
            while (!logCaptured.compare_exchange_weak(expectedFalse, true))
                customSleep<micro>(5);
            printLogDeq();
            if (logTermSignal.load())
            {
                lastLog();
                printLogDeq();
                logStream.close();
                logThreadTerminated.store(true);
                return;
            }
        }
    }).detach();
}

void Engine::Engine::log(const string& msg)
{
    stringstream buffer;
    time_t t = time(nullptr);
    tm* curTime = localtime(&t);
    buffer << '('
           << setfill('0') << setw(2) << curTime->tm_mday << 'd' << ' '
           << setfill('0') << setw(2) << curTime->tm_hour << ':'
           << setfill('0') << setw(2) << curTime->tm_min << ':'
           << setfill('0') << setw(2) << curTime->tm_sec
           << ')';
    buffer << ' ' << msg << '\n';
    while (!logCaptured.compare_exchange_weak(expectedFalse, true))
        customSleep<micro>(10);
    logDeq.push_back(buffer.str());
    logCaptured.store(false);
}

void Engine::Engine::printLogDeq()
{
    while (!logDeq.empty())
    {
        cout << logDeq[0];
        logStream << logDeq[0];
        logDeq.pop_front();
    }
    cout.flush();
    logStream.flush();
}

void Engine::Engine::lastLog()
{
    int64_t uptime = (chrono::time_point_cast<chrono::seconds>(chrono::system_clock::now()) - startPoint).count();
    if (uptime == 0)
        return;
    log(string("Uptime: ") + to_string(uptime) + " seconds");
}

void Engine::Engine::runTimer(std::atomic<bool>& stopFlag, std::atomic<bool>& overFlag, unsigned seconds)
{
    stopFlag = false; overFlag = false;
    std::thread([this, &stopFlag, &overFlag](unsigned seconds)
    {
        unsigned secondsCounter = 0;
        while (true)
        {
            if (terminationSignal.load())
                secondsCounter = seconds - 1;
            log("Remaining for reconnection: " + to_string(seconds - secondsCounter));
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
    log("Try connect to message server");
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
                customSleep<milli>(250);
            } while (!msThreadTerminated.load() && !connected.load());
        }
        while (!msThreadTerminated.load())
            customSleep<milli>(50);
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
            log("Invalid message type");
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
