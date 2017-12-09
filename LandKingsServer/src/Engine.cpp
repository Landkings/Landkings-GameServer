#include "Engine.h"

#include "WsServer.h"

static constexpr uint16_t defaultServerPort = 19999;

Engine::Engine::Engine() : wsServer(*this) {}

void Engine::Engine::run() {
//    scene.addObject((PGameObject)(new Character(&scene, Position(0, 0), "p1.lua")));
//    scene.addObject((PGameObject)(new Character(&scene, Position(0, 10), "p2.lua")));
    scene.addObject(new Character(&scene, Position(250, 245), "p1.lua"));
    scene.addObject(new Character(&scene, Position(450, 310), "p2.lua"));
    auto previous = std::chrono::system_clock::now();
    auto lag = previous - previous;
    int cnt = 0;

    bool serverRunning = true;
    std::thread([this](bool& running)
    {
        running = wsServer.start(defaultServerPort);
    },
    std::ref(serverRunning)).detach();

    std::this_thread::sleep_for(std::chrono::seconds(1));
    if (serverRunning)
        std::cout << "Server +" << std::endl;
    else
    {
        std::cout << "Server -" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::exit(1);
    }

    //std::this_thread::sleep_for(std::chrono::seconds(10));

    while (true)
    {
        //auto current = std::chrono::system_clock::now();
        //auto elapsed = current - previous;
        //previous = xcurrent;
        //lag += elapsed;


        // ***
        waitForMutex(sceneMutex, std::chrono::microseconds(1));
        addPendingPlayers();
        // ***

        //while (lag >= 30) {
            update();
        //    lag -= 30;
        //}

        //scene.print();
        sceneMutex.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
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

// ***

void Engine::Engine::pushPendingPlayer(std::string& nickname, std::string& sourceCode)
{
    waitForMutex(pendingPlayersMutex);
    pendingPlayers.push_back(std::pair<std::string, std::string>(nickname, sourceCode));
    pendingPlayersMutex.unlock();
}

void Engine::Engine::addPendingPlayers()
{
    if (pendingPlayers.size() == 0)
        return;
    waitForMutex(pendingPlayersMutex);
    for (int i = 0; i < pendingPlayers.size(); ++i)
    {
        scene.addPlayer(pendingPlayers[i].first, pendingPlayers[i].second);
                        // TODO: add players into the game
    }
    pendingPlayers.clear();
    pendingPlayersMutex.unlock();
}

// ***
