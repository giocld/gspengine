#pragma once

#include "Types.h"
#include <string>

// Forward declare GLFW
struct GLFWwindow;

namespace gspengine {

    // Instance data for batched sprite rendering
    struct InstanceData {
        float translation[3];
        float scale[2];
    };

    // Uniform data for projection matrix
    struct Uniforms {
        float projection[16];
    };

    class GraphicsManager {
    public:
        GraphicsManager();
        ~GraphicsManager();

        void Startup();
        void Shutdown();
        void Draw();

        bool ShouldClose() const;
        void SetWindowShouldClose(bool value);
        GLFWwindow* GetWindow() const;

    private:
        // Shader utilities
        unsigned int LoadShaderProgram(const std::string& vertPath, const std::string& fragPath);
        unsigned int CompileShader(unsigned int type, const std::string& source);
        std::string ReadFile(const std::string& path);

        // Pipeline initialization
        void InitializePipeline();
        void CreateBuffers();

        // Window
        GLFWwindow* m_window = nullptr;

        // OpenGL objects
        unsigned int m_shaderProgram = 0;
        unsigned int m_vao = 0;
        unsigned int m_vbo = 0;
        unsigned int m_instanceVbo = 0;

        // Projection uniform location
        int m_projectionLoc = -1;
    };
}
