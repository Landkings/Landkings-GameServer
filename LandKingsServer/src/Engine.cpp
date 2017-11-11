#include "../include/Engine.h"

Engine::Engine::Engine() {}

void Engine::Engine::run() {
    while (true) {
        update();

    }
}

void Engine::Engine::update() {
    scene.update();
}
