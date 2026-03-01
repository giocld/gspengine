#pragma once

#include "ResourceManager.h"
#include "SoundManager.h"
#include "Types.h"
#include "GraphicsManager.h"
#include "InputManager.h"
#include "ScriptManager.h"
#include "ResourceManager.h"
#include <functional>

namespace gspengine {

    class Engine {
    public:
        Engine();
        ~Engine();

        void Startup();
        void Shutdown();
        void RunGameLoop(std::function<void()> updateCallback);

        //make managers public so the user can access them (e.g. engine.input.IsKeyPressed)
        GraphicsManager graphics;
        InputManager input;
        ResourceManager resources;
        SoundManager audio;
        ScriptManager scripts;
    };
}
