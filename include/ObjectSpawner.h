#pragma once

#include<random>

#include "Scene.h"
#include "GameObject.h"

namespace Engine {

class ObjectSpawner {
public:
    ObjectSpawner(Scene *scene, GameObject *prototype, Vec2i minBoundaries, Vec2i maxBoundaries, int objectsLimit = -1, int spawnCoolDown = -1) :
        scene(scene),
        prototype(prototype),
        minBoundaries(minBoundaries),
        maxBoundaries(maxBoundaries),
        objectsLimit(objectsLimit),
        objectsCount(0),
        spawnCoolDown(spawnCoolDown),
        lastSpawn(0) {
    }

    template<typename T>
    GameObject* spawn(std::vector<T*> &container) {
        GameObject* newObj = nullptr;
        if ((lastSpawn + spawnCoolDown <= scene->getTime() && objectsCount < objectsLimit) || objectsLimit < 0 || spawnCoolDown < 0) {
            ++objectsCount;
            Vec2i finalPosition = getRandomPosition();
            while (!scene->checkAllCollisions(prototype, &finalPosition))
                finalPosition = getRandomPosition(); //implement better logic
            newObj = prototype->clone();
            newObj->setPosition(finalPosition);
            container.push_back((T*)newObj);
        }
        return newObj;
    }

private:
    Vec2i getRandomPosition();
    Scene* scene;
    GameObject *prototype;
    Vec2i minBoundaries;
    Vec2i maxBoundaries;
    int objectsLimit;
    int objectsCount;
    int spawnCoolDown;
    int lastSpawn;
};

}
