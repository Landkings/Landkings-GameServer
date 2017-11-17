#include "Scene.h"
#include "GameObject.h"

using namespace Engine;

//public methods

Scene::Scene() {
    height = Constants::SCENE_LENGTH;
    width = Constants::SCENE_HEIGHT;
}

void Scene::update() {
    for (auto object : objects) {
        object->update(*this);
    }
}

void Scene::move(GameObject *object, Position &new_pos) {
    if (validPosition(new_pos)) {
        object->setPosition(new_pos);
    }
}

//private methods

bool Scene::validPosition(Position & pos) {
    return pos.getX() >= 0 &&
           pos.getY() >= 0 &&
           pos.getX() <= width &&
           pos.getY() <= height;
}
