#include "../include/GameObject.h"

using namespace Engine;

GameObject::GameObject(Position pos, HitBox hbox) : position(pos), hbox(hbox) {}

Character::Character(Position pos, HitBox hbox) : GameObject(pos, hbox) {}

void Character::move(Scene &scene) {
    scene.move(this);
}

//Obstacle::Obstacle() {}
