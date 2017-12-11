#include "Engine.h"

static constexpr uint16_t defaultWsServerPort = 19998;

Engine::Engine::Engine() {}

void Engine::Engine::run() {
//    scene.addObject((PGameObject)(new Character(&scene, Position(0, 0), "p1.lua")));
//    scene.addObject((PGameObject)(new Character(&scene, Position(0, 10), "p2.lua")));
    scene.addObject(new Character(&scene, Position(250, 245), "p1.lua"));
    scene.addObject(new Character(&scene, Position(450, 310), "p2.lua"));
    auto previous = std::chrono::system_clock::now();
    auto lag = previous - previous;
    int cnt = 0;

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


        // TODO: send objects to WsServer

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
