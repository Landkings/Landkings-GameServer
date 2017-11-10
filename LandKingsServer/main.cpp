#include <iostream>
#include <string>
#include "include/Engine.h"
#include "lib/lua/include/lua.hpp"
#include "lib/lua-wrapper/include/luawrapper.hpp"

using namespace std;

int main(int argc, char* argv[]) {
    Engine::Engine engine;
    engine.run();
    return 0;
}
