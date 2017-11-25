#include <iostream>
#include <string>
#include <thread>

#include "Engine.h"
#include "lua.hpp"
#include "Position.h"
//#include "luawrapper.hpp"

using namespace std;

Engine::Engine* enginePtr;

int main(int argc, char* argv[]) {
    Engine::Engine engine;
    enginePtr = &engine;
    engine.run();
    return 0;
}
