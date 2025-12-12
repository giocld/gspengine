#include "ResourceManager.h"
#include "spdlog/spdlog.h"
#include <filesystem>

namespace gspengine {

    ResourceManager::ResourceManager() {
        //default assumption: assets are in a folder named "assets"
        //relative to the working directory.
        m_rootPath = "assets";
    }

    ResourceManager::~ResourceManager() {
        Shutdown();
    }

    void ResourceManager::Startup() {
        //check if the directory actually exists
        if (!std::filesystem::exists(m_rootPath)) {
            spdlog::warn("ResourceManager: Root path '{}' does not exist!", m_rootPath.string());
        } else {
            spdlog::info("ResourceManager started. Root path: {}", m_rootPath.string());
        }
    }

    void ResourceManager::Shutdown() {
        //nothing special to clean up for now
    }

    void SetRootPath(const std::string& path) {
        //implementation for setting custom root path if needed
        //m_rootPath = path;
    }

    std::string ResourceManager::ResolvePath(const std::string& relativePath) const {
        //combine root path with the requested file
        std::filesystem::path fullPath = m_rootPath / relativePath;
        return fullPath.string();
    }
}
