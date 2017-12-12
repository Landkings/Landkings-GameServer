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

    std::thread([this]()
    {
        auto messageHandler = std::bind(&Engine::Engine::onWsMessage, this,
                                    std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
        wsHub.onMessage(messageHandler);
        std::map<std::string, std::string> header;
        std::getline(std::ifstream("secret.txt"), header["secret"]);
        wsHub.connect("ws://localhost:19998", nullptr, header);
        wsHub.run();
        connected.store(false);
        wsSocket = nullptr;
    }).detach();

    int secondsCounter = 0;
    while (!connected.load())
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        if (++secondsCounter == 5)
        {
            std::cout << "Timeout for connecting" << std::endl;
            return;
        }
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

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

void Engine::Engine::update() {
    scene.update();
}

void Engine::Engine::waitForMutex(std::mutex& m, const std::chrono::microseconds& interval)
{
    while (!m.try_lock())
        std::this_thread::sleep_for(interval);
    return;
}

// **********

void Engine::Engine::onWsMessage(uWS::WebSocket<uWS::CLIENT>* socket, char* message, size_t length, uWS::OpCode opCode)
{
    wsSocket = socket;
    // TODO: wsSocket->send(map); messageType = "loadMap"
    wsSocket->send("{\"messageType\" : \"loadMap\"}"); // del
    connected.store(true);
}

// **********
