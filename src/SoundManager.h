#pragma once

#include "Types.h"
#include "soloud.h"
#include "soloud_wav.h"
#include <string>
#include <unordered_map>

namespace gspengine {

    class SoundManager {
    public:
        SoundManager();
        ~SoundManager();

        void Startup();
        void Shutdown();

        //loads a sound file from disk into memory
        //name:how you want to refer to it
        //path:the full path to the file
        bool LoadSound(const std::string& name, const std::string& path);

        //plays a sound by name
        void PlaySound(const std::string& name);

        //stops a specific sound (optional helper)
        void StopSound(const std::string& name);

    private:
        SoLoud::Soloud m_soloud;
        std::unordered_map<std::string, SoLoud::Wav> m_sounds;
    };
}
