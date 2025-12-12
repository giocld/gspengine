#pragma once

#include "Types.h"
#include <filesystem>
#include <string>

namespace gspengine {

    class ResourceManager {
    public:
        ResourceManager();
        ~ResourceManager();

        void Startup();
        void Shutdown();

        //sets the root directory for assets (e.g. "assets")
        void SetRootPath(const std::string& path);

        //takes a relative path (e.g. "textures/image.png")
        //and returns the full path (e.g. "assets/textures/image.png")
        std::string ResolvePath(const std::string& relativePath) const;

    private:
        std::filesystem::path m_rootPath;
    };
}
