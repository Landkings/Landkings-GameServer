#pragma once
#include <iostream>
#include <chrono>
#include <unistd.h>
#include <thread>
#include <mutex>
#include <functional>
#include <fstream>
#include <atomic>
#include <string>
#include <sstream>

#include <rapidjson/document.h>

#include <uWS/uWS.h>

#include "Scene.h"
#include "GameObject.h"
#include "Vec2i.h"
#include "Log.h"


namespace Engine {


class Engine {
public:
    Engine();
    void run();
    void terminate();
private:
    typedef std::atomic<bool> Flag;
    typedef void (Engine::* MessageProcessor)(char*, size_t);

    enum class OutputMessageType
    {
        unknown = -1, loadMap = 'm', loadObjects = 'o'
    };


    std::array<const MessageProcessor, UCHAR_MAX + 1> MESSAGE_PROCESSOR;

    // ****************

    Scene scene;

    Log log;
    Flag logTermSignal;
    Flag logThreadTerminated;

    uWS::Hub* msHub;
    uWS::WebSocket<uWS::CLIENT>* msSocket;
    Flag connected;
    Flag gameServerKnow;
    Flag messageServerKnow;
    Flag msThreadTerminated;

    Flag terminationSignal;

    std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds> startPoint;

    // *****************

    void update();

    void onMsMessage(uWS::WebSocket<uWS::CLIENT>* socket, char* message, size_t length, uWS::OpCode opCode);
    void processAcceptConnection(char* message, size_t length);
    void processNewPlayer(char* message, size_t length);
    void setMessageType(OutputMessageType type, rapidjson::StringBuffer& buffer);

    bool messageServerConnection();
    void runTimer(std::atomic<bool>& stopFlag, std::atomic<bool>& overFlag, unsigned seconds);
    void runConnection();

    void runLog();
    void lastLog();

    void init();
    void mainLoop();
    void beforeExit();

    template<typename T>
    static void customSleep(unsigned val)
    {
        std::this_thread::sleep_for(std::chrono::duration<int64_t, T>(val));
    }
};

}
