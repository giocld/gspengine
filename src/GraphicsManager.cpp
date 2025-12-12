#include "GraphicsManager.h"
#include <iostream>
#include <fstream>
#include <sstream>

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include "spdlog/spdlog.h"

namespace gspengine {

    GraphicsManager::GraphicsManager() {}
    GraphicsManager::~GraphicsManager() { Shutdown(); }

    void GraphicsManager::Startup() {
        // Initialize GLFW
        if (!glfwInit()) {
            spdlog::error("Failed to initialize GLFW");
            return;
        }

        // Set OpenGL version hints (3.3 Core)
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);

#ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

        // Create window
        m_window = glfwCreateWindow(800, 600, "GSP Engine - OpenGL", nullptr, nullptr);

        if (!m_window) {
            spdlog::error("Failed to create a window.");
            glfwTerminate();
            return;
        }

        // Make context current
        glfwMakeContextCurrent(m_window);

        // Load OpenGL functions using GLAD
        int version = gladLoadGL(glfwGetProcAddress);
        if (version == 0) {
            spdlog::error("Failed to initialize GLAD");
            glfwDestroyWindow(m_window);
            glfwTerminate();
            return;
        }
        spdlog::info("OpenGL loaded: {}.{}", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));

        // Enable vsync
        glfwSwapInterval(1);

        // Set viewport
        int width, height;
        glfwGetFramebufferSize(m_window, &width, &height);
        glViewport(0, 0, width, height);

        // Initialize rendering pipeline
        InitializePipeline();
        CreateBuffers();

        spdlog::info("GraphicsManager started successfully (OpenGL)");
    }

    std::string GraphicsManager::ReadFile(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            spdlog::error("Could not open file: {}", path);
            return "";
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    unsigned int GraphicsManager::CompileShader(unsigned int type, const std::string& source) {
        unsigned int shader = glCreateShader(type);
        const char* src = source.c_str();
        glShaderSource(shader, 1, &src, nullptr);
        glCompileShader(shader);

        // Check for errors
        int success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetShaderInfoLog(shader, 512, nullptr, infoLog);
            spdlog::error("Shader compilation failed: {}", infoLog);
            glDeleteShader(shader);
            return 0;
        }
        return shader;
    }

    unsigned int GraphicsManager::LoadShaderProgram(const std::string& vertPath, const std::string& fragPath) {
        std::string vertSource = ReadFile(vertPath);
        std::string fragSource = ReadFile(fragPath);

        if (vertSource.empty() || fragSource.empty()) {
            return 0;
        }

        unsigned int vertShader = CompileShader(GL_VERTEX_SHADER, vertSource);
        unsigned int fragShader = CompileShader(GL_FRAGMENT_SHADER, fragSource);

        if (vertShader == 0 || fragShader == 0) {
            return 0;
        }

        // Link program
        unsigned int program = glCreateProgram();
        glAttachShader(program, vertShader);
        glAttachShader(program, fragShader);
        glLinkProgram(program);

        // Check for linking errors
        int success;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetProgramInfoLog(program, 512, nullptr, infoLog);
            spdlog::error("Shader program linking failed: {}", infoLog);
            glDeleteProgram(program);
            program = 0;
        }

        // Cleanup shaders (they're linked into the program now)
        glDeleteShader(vertShader);
        glDeleteShader(fragShader);

        return program;
    }

    void GraphicsManager::InitializePipeline() {
        m_shaderProgram = LoadShaderProgram("assets/shaders/shader.vert", "assets/shaders/shader.frag");
        if (m_shaderProgram == 0) {
            spdlog::error("Failed to create shader program");
            return;
        }

        // Get uniform location
        m_projectionLoc = glGetUniformLocation(m_shaderProgram, "uProjection");

        spdlog::info("Shader program initialized successfully");
    }

    void GraphicsManager::CreateBuffers() {
        // Quad vertices: position (x, y) + texcoord (u, v)
        float vertices[] = {
            // pos       // tex
            -1.0f, -1.0f, 0.0f, 1.0f,
             1.0f, -1.0f, 1.0f, 1.0f,
            -1.0f,  1.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 1.0f, 0.0f,
        };

        // Create VAO
        glGenVertexArrays(1, &m_vao);
        glBindVertexArray(m_vao);

        // Create VBO for vertices
        glGenBuffers(1, &m_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        // Position attribute (location = 0)
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // TexCoord attribute (location = 1)
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);

        // Create instance VBO (for future instanced rendering)
        glGenBuffers(1, &m_instanceVbo);
        glBindBuffer(GL_ARRAY_BUFFER, m_instanceVbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(InstanceData) * 100, nullptr, GL_DYNAMIC_DRAW);

        // Translation attribute (location = 2)
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)offsetof(InstanceData, translation));
        glEnableVertexAttribArray(2);
        glVertexAttribDivisor(2, 1); // Per instance

        // Scale attribute (location = 3)
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)offsetof(InstanceData, scale));
        glEnableVertexAttribArray(3);
        glVertexAttribDivisor(3, 1); // Per instance

        glBindVertexArray(0);

        spdlog::info("GPU Buffers created successfully");
    }

    void GraphicsManager::Draw() {
        glfwPollEvents();

        // Handle resize
        int width, height;
        glfwGetFramebufferSize(m_window, &width, &height);
        if (width == 0 || height == 0) return;

        glViewport(0, 0, width, height);

        // Clear with green (same as before)
        glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Use shader and draw
        if (m_shaderProgram != 0) {
            glUseProgram(m_shaderProgram);

            // Simple orthographic projection (identity for now)
            float projection[16] = {
                1.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f
            };
            if (m_projectionLoc >= 0) {
                glUniformMatrix4fv(m_projectionLoc, 1, GL_FALSE, projection);
            }

            glBindVertexArray(m_vao);
            // Just clear for now - no actual geometry drawn yet
            glBindVertexArray(0);
        }

        glfwSwapBuffers(m_window);
    }

    void GraphicsManager::Shutdown() {
        if (m_instanceVbo) {
            glDeleteBuffers(1, &m_instanceVbo);
            m_instanceVbo = 0;
        }
        if (m_vbo) {
            glDeleteBuffers(1, &m_vbo);
            m_vbo = 0;
        }
        if (m_vao) {
            glDeleteVertexArrays(1, &m_vao);
            m_vao = 0;
        }
        if (m_shaderProgram) {
            glDeleteProgram(m_shaderProgram);
            m_shaderProgram = 0;
        }

        if (m_window) {
            glfwDestroyWindow(m_window);
            m_window = nullptr;
        }
        glfwTerminate();
        spdlog::info("GraphicsManager shut down");
    }

    bool GraphicsManager::ShouldClose() const {
        return m_window && glfwWindowShouldClose(m_window);
    }

    void GraphicsManager::SetWindowShouldClose(bool value) {
        if (m_window) glfwSetWindowShouldClose(m_window, value);
    }

    GLFWwindow* GraphicsManager::GetWindow() const {
        return m_window;
    }
}
