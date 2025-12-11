#include <iostream>
#include "Engine.h"

// Define a simple update function for our game
void MyGameUpdate() {
    static int frames = 0;
    frames++;
    if (frames % 60 == 0) {
        std::cout << "Tick: " << frames << std::endl;
    }
}

int main() {
    std::cout << "Starting Game..." << std::endl;

    gspengine::Engine engine;
    engine.Startup();
    engine.RunGameLoop(MyGameUpdate);
    engine.Shutdown();
    
    return 0;
}
