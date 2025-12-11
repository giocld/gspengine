#pragma once

#include "Types.h"
#include "GraphicsManager.h"
#include <functional>

namespace gspengine {

    class Engine {
    public:
        Engine();
        ~Engine();

        void Startup();
        void Shutdown();
        void RunGameLoop(std::function<void()> updateCallback);

    private:
        // the engine owns the graphics manager
        GraphicsManager m_graphics;
    };
}
