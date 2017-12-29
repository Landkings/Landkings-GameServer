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

#include "document.h"

#include <uWS/uWS.h>

#include "Scene.h"
#include "GameObject.h"
#include "Position.h"


namespace Engine {


class Engine {
public:
    Engine();
    void run();
    void terminate();
private:
    typedef std::atomic<bool> Flag;

    enum class InputMessageType
    {
        unknown = -1, acceptConnection = 'c', newPlayer = 'p'
    };

    enum class OutputMessageType
    {
        unknown = -1, loadMap = 'm', loadObjects = 'o'
    };

    // *******************

    static bool expectedFalse;

    // *******************

    void update();

    void onMsMessage(uWS::WebSocket<uWS::CLIENT>* socket, char* message, size_t length, uWS::OpCode opCode);
    void processAcceptConnection();
    void processNewPlayer(const char* message, size_t length);
    InputMessageType getMessageType(char firstChar) const;
    void setMessageType(OutputMessageType type, rapidjson::StringBuffer& buffer);

    bool messageServerConnection();
    void runTimer(std::atomic<bool>& stopFlag, std::atomic<bool>& overFlag, unsigned seconds);
    void runConnection();

    void runLog();
    void log(const std::string& msg);
    void printLogDeq();
    void lastLog();

    void init();
    void mainLoop();
    void beforeExit();

    template<typename T>
    static void customSleep(unsigned val)
    {
        std::this_thread::sleep_for(std::chrono::duration<int64_t, T>(val));
    }

    // *********************

    Scene scene;

    std::ofstream logStream;
    std::deque<std::string> logDeq;
    Flag logCaptured;
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
};

}
