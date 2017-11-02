#include "GameObject.h"

GameObject::GameObject() {}

Point GameObject::getPosition() {
    return position;
}

std::string GameObject::getName() {
    return name;
}

Character::Character() {}

Obstacle::Obstacle() {}
