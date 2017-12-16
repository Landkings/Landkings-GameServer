#include "Engine.h"

static constexpr uint16_t defaultWsServerPort = 19998;

Engine::Engine::Engine() : wsSocket(nullptr) {
    connected.store(false);
}

void Engine::Engine::run() {
//    scene.addObject((PGameObject)(new Character(&scene, Position(0, 0), "p1.lua")));
//    scene.addObject((PGameObject)(new Character(&scene, Position(0, 10), "p2.lua")));
    scene.addObject(new Character(&scene, Position(250, 245), "p1.lua"));
    scene.addObject(new Character(&scene, Position(450, 310), "p2.lua"));
    auto previous = std::chrono::system_clock::now();
    auto lag = previous - previous;
    int cnt = 0;

    if (!messageServerConnection())
    {
        std::cout << "Timeout for connection" << std::endl;
        return;
    }
    while (true)
    {
        //auto current = std::chrono::system_clock::now();
        //auto elapsed = current - previous;
        //previous = xcurrent;
        //lag += elapsed;

        //while (lag >= 30) {
            update();
        //    lag -= 30;
        //}

        //scene.print();

        // TODO: wsSocket->send(objects); messageType = "loadObjects"
        if (connected.load())
            wsSocket->send("{\"messageType\" : \"loadObjects\"}"); // del
        else
        {
            delete wsHub;
            if (messageServerConnection())
                continue;
            std::cout << "Timeout for connection" << std::endl;
            return;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

void Engine::Engine::update() {
    scene.update();
}


// **********


bool Engine::Engine::messageServerConnection()
{
    std::atomic<bool> timeout, stopTimer;
    timeout = false; stopTimer = false;
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

void Engine::Engine::onWsMessage(uWS::WebSocket<uWS::CLIENT>* socket, char* message, size_t length, uWS::OpCode opCode) {
    wsSocket = socket;
    // TODO: wsSocket->send(map); messageType = "loadMap"
    wsSocket->send("{\"messageType\" : \"loadMap\"}"); // del
    connected.store(true);
}

// **********
