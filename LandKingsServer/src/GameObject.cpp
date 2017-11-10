#include "../include/GameObject.h"

using namespace Engine;

GameObject::GameObject(Position pos, HitBox hbox) : position(pos), hbox(hbox) {}

Character::Character(Position pos, HitBox hbox) : GameObject(pos, hbox) {}

//Obstacle::Obstacle() {}
