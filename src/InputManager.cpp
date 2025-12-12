#include "InputManager.h"

namespace gspengine {

    void InputManager::Startup(GLFWwindow* window) {
        m_window = window;
    }

    void InputManager::Shutdown() {
        //GLFW termination is handled by GraphicsManager
    }

    void InputManager::Update() {
        //process all pending events (mouse moved, keys pressed, etc.)
        glfwPollEvents();
    }

    bool InputManager::IsKeyPressed(int key) {
        if (!m_window) return false;
        return glfwGetKey(m_window, key) == GLFW_PRESS;
    }
}
