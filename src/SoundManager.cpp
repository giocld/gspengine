#include "SoundManager.h"
#include "spdlog/spdlog.h"

namespace gspengine {

    SoundManager::SoundManager() {
        //constructor
    }

    SoundManager::~SoundManager() {
        Shutdown();
    }

    void SoundManager::Startup() {
        //initialize the SoLoud engine
        //defaults are usually fine
        m_soloud.init();
        spdlog::info("SoundManager started");
    }

    void SoundManager::Shutdown() {
        //stop all sounds and clean up
        m_soloud.deinit();
        m_sounds.clear();
        spdlog::info("SoundManager shut down");
    }

    bool SoundManager::LoadSound(const std::string& name, const std::string& path) {
        //load the wav file (SoLoud handles wav ogg mp3 via the Wav class)
        SoLoud::result res = m_sounds[name].load(path.c_str());

        if (res != SoLoud::SO_NO_ERROR) {
            spdlog::error("Failed to load sound '{}' from path: {}", name, path);
            m_sounds.erase(name); //clean up empty entry
            return false;
        }

        spdlog::info("Loaded sound '{}'", name);
        return true;
    }

    void SoundManager::PlaySound(const std::string& name) {
        if (m_sounds.find(name) != m_sounds.end()) {
            //play the sound (returns a handle)
            m_soloud.play(m_sounds[name]);
        } else {
            spdlog::warn("Attempted to play undefined sound '{}'", name);
        }
    }

    void SoundManager::StopSound(const std::string& name) {
        //optional implementation
    }
}
