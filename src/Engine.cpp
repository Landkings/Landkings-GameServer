#include "Engine.h"

#include <iomanip>

#include <rapidjson/stringbuffer.h>

using namespace rapidjson;
using namespace uWS;
using namespace std;


static constexpr uint16_t defaultMsPort = 19998;
static constexpr unsigned defaultReconnectionTime = 25;
static constexpr unsigned defaultLogInterval = 250;


Engine::Engine::Engine() : MESSAGE_PROCESSOR{} {
    const_cast<MessageProcessor&>(MESSAGE_PROCESSOR['c']) = &Engine::processAcceptConnection;
    const_cast<MessageProcessor&>(MESSAGE_PROCESSOR['p']) = &Engine::processNewPlayer;
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
    logTermSignal = false;
    logThreadTerminated = false;
    connected = false;
    gameServerKnow = false;
    messageServerKnow = false;
    runLog();
    customSleep<milli>(100);
    if (!messageServerConnection())
        log.write("Can't connect");
}

std::string Engine::Engine::readCode(std::string fileName) {
    std::ifstream input(fileName, std::ios::binary);
    std::stringstream luaCode;
    luaCode << input.rdbuf();
    return luaCode.str();
}

void Engine::Engine::mainLoop()
{
//    scene.addObject(new Character(&scene, "p1.lua", "ilya", Vec2i(20, 20)));
    std::srand(unsigned(std::time(0)));
    scene.addPlayer("John", readCode("p2.lua"));;
    int countPfM = 5; // TODO : random count monsters
    for (int i = 0; i < countPfM; i++) {
        scene.spawnCharacter("peasful monster", readCode("p1.lua"));
    }
    auto previous = std::chrono::system_clock::now();
    auto lag = previous - previous;
    int cnt = 0;
    int ticks = 0;
    int ctr = 0;
    std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> _startPoint;
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
            log.write("Connection lost");
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
    thread([this]()
    {
        while (true)
        {
            customSleep<milli>(defaultLogInterval);
            log.flush();
            if (logTermSignal.load())
            {
                lastLog();
                log.flush();
                logThreadTerminated.store(true);
                return;
            }
        }
    }).detach();
}

void Engine::Engine::lastLog()
{
    int64_t uptime = (chrono::time_point_cast<chrono::seconds>(chrono::system_clock::now()) - startPoint).count();
    if (uptime == 0)
        return;
    log.write(string("Uptime: ") + to_string(uptime) + " seconds");
}

void Engine::Engine::runTimer(Flag& stopFlag, Flag& overFlag, unsigned seconds)
{
    stopFlag = false; overFlag = false;
    std::thread([this, &stopFlag, &overFlag](unsigned seconds)
    {
        unsigned secondsCounter = 0;
        while (true)
        {
            if (terminationSignal.load())
                secondsCounter = seconds - 1;
            log.write("Remaining for rc: " + to_string(seconds - secondsCounter));
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
        msHub->connect("ws://localhost:19998", nullptr, header);// defaultReconnectionTime);
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
    log.write("Try connect to message server");
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
    msSocket = socket;
    MessageProcessor prc = MESSAGE_PROCESSOR[static_cast<unsigned char>(message[0])];
    if (prc)
        (this->*prc)(message + 1, length - 1);
    else
        log.write("Invalid message type");
}

void Engine::Engine::processAcceptConnection(char* message, size_t length)
{
    connected.store(true);
    StringBuffer buffer;
    setMessageType(OutputMessageType::loadMap, buffer);
    scene.createMapMessage(buffer);
    msSocket->send(buffer.GetString(), buffer.GetLength(), TEXT);
}

void Engine::Engine::processNewPlayer(char* message, size_t length)
{
    string m(message, length);
    int idx = m.find_first_of('>');
    log.write(string("New player:") + " nick = " + m.substr(0, idx) + " | code =\n" + m.substr(idx + 1));
    scene.addPlayer(m.substr(0, idx), m.substr(idx + 1));
}

void Engine::Engine::setMessageType(OutputMessageType type, StringBuffer& buffer)
{
    buffer.Put(static_cast<char>(type));
}
