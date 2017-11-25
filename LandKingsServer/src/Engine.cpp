#include "Engine.h"

Engine::Engine::Engine() {}

void Engine::Engine::run() {
//    scene.addObject((PGameObject)(new Character(&scene, Position(0, 0), "p1.lua")));
//    scene.addObject((PGameObject)(new Character(&scene, Position(0, 10), "p2.lua")));
    scene.addObject(new Character(&scene, Position(0, 5), "p1.lua"));
    scene.addObject(new Character(&scene, Position(0, 10), "p2.lua"));
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
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void Engine::Engine::update() {
    scene.update();
}
