#include "Engine.h"
#include <chrono>
#include <thread>

namespace gspengine {

    Engine::Engine() { }

    Engine::~Engine() { Shutdown(); }

    void Engine::Startup() {
        // start up managers in order
        m_graphics.Startup();
    }

    void Engine::Shutdown() {
        // shut down managers in reverse order
        m_graphics.Shutdown();
    }

    void Engine::RunGameLoop(std::function<void()> updateCallback) {
        using namespace std::chrono;
        const auto SECONDS_PER_TICK = duration<real>(1.0 / 60.0);
        auto last_tick = steady_clock::now();

        // loop until the window says it should close
        while (!m_graphics.ShouldClose()) {
            auto now = steady_clock::now();

            // catch up logic (fixed time step)
            while (now - last_tick >= SECONDS_PER_TICK) {
                if (updateCallback) {
                    updateCallback();
                }
                last_tick += duration_cast<steady_clock::duration>(SECONDS_PER_TICK);
            }

            // draw frame and poll events
            m_graphics.Draw();
            
            // yield to cpu to save power
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}
