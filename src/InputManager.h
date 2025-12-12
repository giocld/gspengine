#pragma once

#include "Types.h"

//we expose GLFW headers so users can use key constants (e.g. GLFW_KEY_SPACE)
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace gspengine {

    class InputManager {
    public:
        //inputManager needs the window to check for key presses
        void Startup(GLFWwindow* window);
        void Shutdown();

        //this will call glfwPollEvents()
        void Update();

        //check if a specific key is being held down
        bool IsKeyPressed(int key);

    private:
        GLFWwindow* m_window = nullptr;
    };
}
