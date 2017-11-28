#include "Engine.h"

#include "WsServer.h"

Engine::Engine* enginePtr;

static constexpr uint16_t defaultServerPort = 19999;

Engine::Engine::Engine() {}

void Engine::Engine::run() {
//    scene.addObject((PGameObject)(new Character(&scene, Position(0, 0), "p1.lua")));
//    scene.addObject((PGameObject)(new Character(&scene, Position(0, 10), "p2.lua")));
    scene.addObject(new Character(&scene, Position(250, 245), "p1.lua"));
    scene.addObject(new Character(&scene, Position(450, 310), "p2.lua"));
    auto previous = std::chrono::system_clock::now();
    auto lag = previous - previous;
    int cnt = 0;

    bool serverRunning = true;
    std::thread([](bool& running)
    {
        WsServer server;
        running = server.start(defaultServerPort);
        if (!running)
        {
            int s = server.errorCounter();
            for (int i = 0; i < s; ++i)
                std::cout << server.error();
        }
    },
    std::ref(serverRunning)).detach();

    std::this_thread::sleep_for(std::chrono::seconds(1));
    if (serverRunning)
        std::cout << "Server +" << std::endl;
    else
    {
        std::cout << "Server -" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(3));
        std::exit(1);
    }

    // test server output

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
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

void Engine::Engine::update() {
    scene.update();
}

const Engine::Scene& Engine::Engine::getScene() const
{
    return scene;
}
