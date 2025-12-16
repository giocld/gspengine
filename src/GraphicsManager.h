#pragma once

#include "Types.h"
#include <string>
#include <unordered_map>
#include <vector>

//forward declare GLFW
struct GLFWwindow;

namespace gspengine {

    //data for batched sprite rendering
    struct InstanceData {
        float translation[3];
        float scale[2];
    };

    //data for projection matrix
    struct Uniforms {
        float projection[16];
    };
    struct TextureData {
      unsigned int id = 0;
      int width = 0;
      int height = 0;
    };

    struct Sprite {
      std::string textureName;
      float x = 0.0f;     //pos x
      float y = 0.0f;     //pos y
      float scaleX = 1.0f;//scale width
      float scaleY = 1.0f;//scale height
      float z = 0.0f;     //depth, for sorting, higher = in front
    };

    class GraphicsManager {
    public:
        GraphicsManager();
        ~GraphicsManager();

        void Startup();
        void Shutdown();
        void Draw();
        void AddSprite(const Sprite& sprite);

        bool ShouldClose() const;
        void SetWindowShouldClose(bool value);
        GLFWwindow* GetWindow() const;

        bool LoadTexture(const std::string& name, const std::string& path);
    private:
        //shader utilities
        unsigned int LoadShaderProgram(const std::string& vertPath, const std::string& fragPath);
        unsigned int CompileShader(unsigned int type, const std::string& source);
        std::string ReadFile(const std::string& path);

        //init pipeline
        void InitializePipeline();
        void CreateBuffers();

        //window
        GLFWwindow* m_window = nullptr;

        //opengl objects
        unsigned int m_shaderProgram = 0;
        unsigned int m_vao = 0;
        unsigned int m_vbo = 0;
        unsigned int m_instanceVbo = 0;

        //projection uniform location
        int m_projectionLoc = -1;
        int m_textureLoc = -1;
        std::unordered_map<std::string, TextureData> m_textures;

        // Sprites to draw this frame (cleared after each Draw)
        std::vector<Sprite> m_spritesToDraw;
    };
}
