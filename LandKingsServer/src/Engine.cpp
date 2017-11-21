#include "Engine.h"

Engine::Engine::Engine() {}

void Engine::Engine::run() {
    scene.addObject((PGameObject)(new Character(&scene)));
    auto previous = std::chrono::system_clock::now();
    auto lag = previous - previous;
    int cnt = 0;
 	while (true) {
        //auto current = std::chrono::system_clock::now();
        //auto elapsed = current - previous;
        //previous = current;
        //lag += elapsed;
        //while (lag >= 30) {
            update();
        //    lag -= 30;
        //}

        scene.print();
        Sleep(100); // TODO: replace sleep with cross platform
        //std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void Engine::Engine::update() {
    scene.update();
}
