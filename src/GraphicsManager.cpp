#include "GraphicsManager.h"
#include <iostream>
#include <fstream>
#include <sstream>

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <string>
#include <unordered_map>
#include <vector>
#include "spdlog/spdlog.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


namespace gspengine {

    GraphicsManager::GraphicsManager() {}
    GraphicsManager::~GraphicsManager() { Shutdown(); }

    void GraphicsManager::Startup() {
        //initi glfw
      if (!glfwInit()) {
            spdlog::error("Failed to initialize GLFW");
            return;
        }

        //set opengl core version, core 3.3
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);

#ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

        //start the window
        m_window = glfwCreateWindow(800, 600, "GSP Engine - OpenGL", nullptr, nullptr);

        if (!m_window) {
            spdlog::error("Failed to create a window.");
            glfwTerminate();
            return;
        }

        //make the context current
        glfwMakeContextCurrent(m_window);

        //glad, we load opengl
        int version = gladLoadGL(glfwGetProcAddress);
        if (version == 0) {
            spdlog::error("Failed to initialize GLAD");
            glfwDestroyWindow(m_window);
            glfwTerminate();
            return;
        }
        spdlog::info("OpenGL loaded: {}.{}", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));

        //vsync on
        glfwSwapInterval(1);

        //set viewpoint
        int width, height;
        glfwGetFramebufferSize(m_window, &width, &height);
        glViewport(0, 0, width, height);

       //init pipeline and make buffers
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

        //check the compile status for error
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

        //link program
        unsigned int program = glCreateProgram();
        glAttachShader(program, vertShader);
        glAttachShader(program, fragShader);
        glLinkProgram(program);

        //check for link errors
        int success;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetProgramInfoLog(program, 512, nullptr, infoLog);
            spdlog::error("Shader program linking failed: {}", infoLog);
            glDeleteProgram(program);
            program = 0;
        }

        //clean the shaders, now that theyre linked into the program
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

        //get uniform location
        m_projectionLoc = glGetUniformLocation(m_shaderProgram, "uProjection");
        m_textureLoc = glGetUniformLocation(m_shaderProgram, "uTexture");
        spdlog::info("Shader program initialized successfully");
    }

    void GraphicsManager::CreateBuffers() {
        //quad vertices: position (x, y) + texcoord (u, v)
        float vertices[] = {
            // pos       // tex
            -1.0f, -1.0f, 0.0f, 1.0f,
             1.0f, -1.0f, 1.0f, 1.0f,
            -1.0f,  1.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 1.0f, 0.0f,
        };

        //create vao, vertex array object to store vertex attribute confs and bindings from buffers
        glGenVertexArrays(1, &m_vao);
        glBindVertexArray(m_vao);

        //create vbo to pass the vertex data to the gpu
        glGenBuffers(1, &m_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        //position attribute, loc = 0
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        //texcoord attribute loc = 1
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);

        //instance vbo (for future instanced rendering)
        glGenBuffers(1, &m_instanceVbo);
        glBindBuffer(GL_ARRAY_BUFFER, m_instanceVbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(InstanceData) * 100, nullptr, GL_DYNAMIC_DRAW);

        //translation attribute loc = 2
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)offsetof(InstanceData, translation));
        glEnableVertexAttribArray(2);
        glVertexAttribDivisor(2, 1); //per instance

        //scale attribute loc = 3
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)offsetof(InstanceData, scale));
        glEnableVertexAttribArray(3);
        glVertexAttribDivisor(3, 1); //per instance aswell

        glBindVertexArray(0);

        spdlog::info("GPU Buffers created successfully");
    }

    bool GraphicsManager::LoadTexture(const std::string& name, const std::string& path) {
        // Load image with stb_image
        int width, height, channels;
        unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 4);

        if (!data) {
            spdlog::error("Failed to load texture: {}", path);
            return false;
        }

        // Create OpenGL texture
        unsigned int textureId;
        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);

        // Set texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Upload pixel data to GPU
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

        // Free CPU memory
        stbi_image_free(data);

        // Store in our map
        m_textures[name] = TextureData{textureId, width, height};

        spdlog::info("Loaded texture '{}' ({}x{})", name, width, height);
        return true;
    }

    void GraphicsManager::Draw() {
        const auto& sprites = m_spritesToDraw;  // Use queued sprites

        //handle resize
        int width, height;
        glfwGetFramebufferSize(m_window, &width, &height);
        if (width == 0 || height == 0) return;

        glViewport(0, 0, width, height);

        //dark blue window
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        //use the shader and draw
        if (m_shaderProgram != 0) {
           glUseProgram(m_shaderProgram);

    //set up orthographic projection (screen coords: 0,0 = bottom-left)
    float projection[16] = {
    2.0f/width, 0.0f,        0.0f, 0.0f,
    0.0f,       2.0f/height, 0.0f, 0.0f,
    0.0f,       0.0f,        1.0f, 0.0f,
    -1.0f,      -1.0f,        0.0f, 1.0f
};

if (m_projectionLoc >= 0) {
    glUniformMatrix4fv(m_projectionLoc, 1, GL_FALSE, projection);
}

//enable blending for transparency
glEnable(GL_BLEND);
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

glBindVertexArray(m_vao);

std::unordered_map<std::string, std::vector<Sprite>> batches;

for (const auto& sprite : sprites) {
    auto it = m_textures.find(sprite.textureName);
    if (it == m_textures.end()) {
        spdlog::warn("Sprite references unknown texture: '{}'", sprite.textureName);
        continue;
    }
    batches[sprite.textureName].push_back(sprite);
}

spdlog::debug("Processing {} sprites into {} batches", sprites.size(), batches.size());

for (const auto& [textureName, spriteGroup] : batches){
    auto it = m_textures.find(textureName);
    if(it == m_textures.end()) continue;
    const TextureData& tex = it->second;

    spdlog::debug("Drawing batch '{}' with {} sprites", textureName, spriteGroup.size());

    //bind texture ONCE for each of the batches
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex.id);
    if (m_textureLoc >= 0) {
        glUniform1i(m_textureLoc, 0);
    }
    
    std::vector<InstanceData> instances;
    instances.reserve(spriteGroup.size());

    for (const auto& sprite : spriteGroup) {
        InstanceData instance;
        instance.translation[0] = sprite.x;
        instance.translation[1] = sprite.y;
        instance.translation[2] = sprite.z;
        instance.scale[0] = sprite.scaleX * tex.width;
        instance.scale[1] = sprite.scaleY * tex.height;
        instances.push_back(instance);
        spdlog::trace("  Sprite at ({}, {}), scale ({}, {})", 
                      sprite.x, sprite.y, instance.scale[0], instance.scale[1]);
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, m_instanceVbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(InstanceData) * instances.size(), instances.data());
    
    //we draw all sprites of a batch in 1 instance
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, instances.size());
    spdlog::debug("  Drew {} instances", instances.size());
}
glBindVertexArray(0);
        }

        glfwSwapBuffers(m_window);
        m_spritesToDraw.clear();  // Clear for next frame
    }

    void GraphicsManager::AddSprite(const Sprite& sprite) {
        m_spritesToDraw.push_back(sprite);
    }

    void GraphicsManager::Shutdown() {
        //we delete textures to free gpu memory
      for (auto& [name, tex] : m_textures){
        if (tex.id != 0){
          glDeleteTextures(1, &tex.id);
        }
      }
      m_textures.clear();

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
