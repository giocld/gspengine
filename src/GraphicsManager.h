#pragma once

#include "Types.h"

// forward declaration so we don't need the heavy glfw header here
struct GLFWwindow; 

namespace gspengine {
    
    class GraphicsManager {
    public:
        GraphicsManager();
        ~GraphicsManager();

        void Startup();
        void Shutdown();
        
        // polls events and swaps buffers (later)
        void Draw(); 
        
        // returns true if the user tried to close the window
        bool ShouldClose() const;

    private:
        GLFWwindow* m_window = nullptr;
    };
}
