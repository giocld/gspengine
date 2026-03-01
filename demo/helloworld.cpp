#include <iostream>
#include <ostream>
#include "Engine.h"

//we need a pointer to the engine so the callback can access it.
//in a real game, we would use a class for the game app.
gspengine::Engine* g_Engine = nullptr;

void MyGameUpdate() {
    //check for Exit
    if (g_Engine->input.IsKeyPressed(GLFW_KEY_ESCAPE)) {
        g_Engine->graphics.SetWindowShouldClose(true);
    }

    //check for spacebar
    static bool spaceWasPressed = false;
    if (g_Engine->input.IsKeyPressed(GLFW_KEY_SPACE)) {
        if (!spaceWasPressed) {
            std::cout << "Spacebar pressed!" << std::endl;
            spaceWasPressed = true;
        }
    } else {
        spaceWasPressed = false;
    }
   static bool checkedPath = false;
   if  (!checkedPath){
     std::string path = g_Engine->resources.ResolvePath("textures/hero.png");
     std::cout << "Resolved Path: " << path << std::endl;
     checkedPath = true;
   }
    static bool enterPressed = false;
    if (g_Engine->input.IsKeyPressed(GLFW_KEY_ENTER)) {
        if (!enterPressed) {
            std::cout << "Playing Sound!" << std::endl;
            g_Engine->audio.PlaySound("mysound");
            enterPressed = true;
        }
    } else {
        enterPressed = false;
    }

    // Draw the hero sprite every frame
    gspengine::Sprite hero;
    hero.textureName = "test";  // Matches the name in LoadTexture
    hero.x = 400.0f;            // Center-ish X
    hero.y = 300.0f;            // Center-ish Y
    hero.scaleX = 0.5f;         // Half size
    hero.scaleY = 0.5f;
    g_Engine->graphics.AddSprite(hero);
}

int main() {
    std::cout << "Starting Game..." << std::endl;

    gspengine::Engine engine;
    g_Engine = &engine; //set the global pointer
    engine.Startup();
    engine.graphics.LoadTexture("test", engine.resources.ResolvePath("textures/hero.png"));
//--------------------------------------------------------------------------
// 1. Resolve the path
    std::string soundPath = engine.resources.ResolvePath("sounds/test.mp3");

// 2. Load the sound
    if (engine.audio.LoadSound("startup_sound", soundPath)) {
        std::cout << "Sound loaded successfully! Playing now..." << std::endl;

// 3. PLAY IMMEDIATELY (No key press needed)
        engine.audio.PlaySound("startup_sound"); } else {
        std::cout << "ERROR: Could not load sound at: " << soundPath << std::endl;
        std::cout << "Make sure you have a file at 'assets/sounds/test.mp3'" << std::endl;
    }
//----------------------------------------------------------------------------
    engine.RunGameLoop(MyGameUpdate);
    engine.Shutdown();
    return 0;
}
