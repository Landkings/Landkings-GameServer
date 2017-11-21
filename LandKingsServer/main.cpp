#include <iostream>
#include <string>
#include <thread>

#include "Engine.h"
#include "lua.hpp"
//#include "luawrapper.hpp"

using namespace std;

int main(int argc, char* argv[]) {
    Engine::Engine engine;
    engine.run();
    return 0;
}
