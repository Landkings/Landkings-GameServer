#include "../include/GameObject.h"

using namespace Engine;

// Game Object

GameObject::GameObject(Position pos, HitBox hbox) : position(pos), hbox(hbox) {}

// Character

// public methods

Character::Character(Position pos, HitBox hbox) : GameObject(pos, hbox) {}

void Character::update(Scene &scene) {
    move(scene);
}

// private methods

void Character::move(Scene &scene) {
    Position direction[4] = {
        Position(1, 0),
        Position(0, 1),
        Position(-1,0),
        Position(0, -1)
    };
    scene.move(this, position + direction[0] * speed);
}

//Obstacle::Obstacle() {}
