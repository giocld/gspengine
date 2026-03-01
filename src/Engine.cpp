#include "Engine.h"
#include <chrono>
#include <thread>

namespace gspengine {

    Engine::Engine() { }

    Engine::~Engine() { Shutdown(); }

    void Engine::Startup() {
    //we start:
        graphics.Startup(); //graphics
        input.Startup(graphics.GetWindow()); //input
        resources.Startup(); //resources
        audio.Startup(); //audio
        scripts.Init(); //scripts
    }

    void Engine::Shutdown() {
        scripts.Shutdown();
        input.Shutdown();
        graphics.Shutdown();
        resources.Shutdown();
        audio.Shutdown();
    }

    void Engine::RunGameLoop(std::function<void()> updateCallback) {
        using namespace std::chrono;
        const auto SECONDS_PER_TICK = duration<real>(1.0 / 60.0);
        auto last_tick = steady_clock::now();

        while (!graphics.ShouldClose()) {
            auto now = steady_clock::now();

            while (now - last_tick >= SECONDS_PER_TICK) {
                //poll events before running game logic so keys are fresh
                input.Update();

                //run game logic
                if (updateCallback) {
                    updateCallback();
                }

                last_tick += duration_cast<steady_clock::duration>(SECONDS_PER_TICK);
            }

            graphics.Draw();

            //sleep to save CPU
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}
