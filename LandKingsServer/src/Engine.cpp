#include "Engine.h"

Engine::Engine::Engine() {}

void Engine::Engine::run() {
	GameObject *character = new Character(&scene);
 	while (true) {
        update();
    }
}

void Engine::Engine::update() {
    scene.update();
}
