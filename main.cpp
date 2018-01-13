#include <iostream>
#include <string>
#include <thread>

#include "Engine.h"
#include "lua.hpp"
#include "Vec2i.h"
//#include "luawrapper.hpp"

using namespace std;


Engine::Engine* enginePtr;

void sigIntHandler(int)
{
    enginePtr->terminate();
    cout << "\nSuccess termination\n" << endl;
    exit(0);
}

void setSigIntHandler()
{
    struct sigaction* sigAction = new struct sigaction;
    sigAction->sa_handler = &sigIntHandler;
    sigAction->sa_flags = 0;
    sigaction(SIGINT, sigAction, NULL);
}

int main(int argc, char* argv[]) {
    Engine::Engine engine;
    enginePtr = &engine;
    setSigIntHandler();
    engine.run();
    return 0;
}
