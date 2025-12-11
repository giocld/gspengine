#include "GraphicsManager.h"
#include <iostream>

// include glfw only here in the implementation
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

// include spdlog
#include "spdlog/spdlog.h"

namespace gspengine {

    GraphicsManager::GraphicsManager() {
        // constructor defaults
    }

    GraphicsManager::~GraphicsManager() {
        // ensure cleanup happens if the user forgets to call shutdown
        Shutdown();
    }

    void GraphicsManager::Startup() {
        if (!glfwInit()) {
            spdlog::error("Failed to initialize GLFW");
            return;
        }

        // we don't want glfw to set up a graphics api (since we will use webgpu later)
         glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        // create the window (800x600 resolution)
        const int width = 800;
        const int height = 600;
        m_window = glfwCreateWindow(width, height, "GSP Engine Window", nullptr, nullptr);
        
        if (!m_window) {
            spdlog::error("Failed to create a window.");
            glfwTerminate();
            return;
        }

        // lock aspect ratio so resizing doesn't stretch things weirdly
        glfwSetWindowAspectRatio(m_window, width, height);
        spdlog::info("GraphicsManager started successfully");
    }

    void GraphicsManager::Shutdown() {
        if (m_window) {
            glfwDestroyWindow(m_window);
            m_window = nullptr;
        }
        glfwTerminate();
        spdlog::info("GraphicsManager shut down");
    }

    void GraphicsManager::Draw() {
        // process window events (mouse clicks, key presses, close button)
        // if we don't do this, the window will freeze
        glfwPollEvents();
    }

    bool GraphicsManager::ShouldClose() const {
        return m_window && glfwWindowShouldClose(m_window);
    }
}
