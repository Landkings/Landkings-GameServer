#include <iostream>
#include <string>
#include <thread>

#include "Engine.h"
#include "lua.hpp"
#include "Position.h"
//#include "luawrapper.hpp"

using namespace std;

int main(int argc, char* argv[]) {
    Engine::Engine engine;
    engine.run();
    cout << "vse norm" << endl;
    return 0;
}
