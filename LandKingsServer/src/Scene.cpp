#include "Scene.h"
#include "GameObject.h"
using namespace Engine;

Scene::Scene() {
    length = Constants::SCENE_LENGTH;
    width = Constants::SCENE_HEIGHT;
}

void Scene::move(GameObject* object) {
    object->getPosition();
}

void Scene::update() {
    for (auto object: objects) {

    }
}
