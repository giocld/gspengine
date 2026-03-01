#pragma once
#include <string>
#include <sol/sol.hpp>
#include <spdlog/spdlog.h>
#include <utility>

namespace gspengine {
  class ScriptManager {
  public:
    ScriptManager();
    ~ScriptManager();


    bool Init();
    void Shutdown();

    bool LoadScript(const std::string& path);
    bool ExecuteString(const std::string& code);

    template<typename... Args>
    auto CallFunction(const std::string& func, Args&&... args) {
      try {
        return lua_[func](std::forward<Args>(args)...);
      } catch (const sol::error& e) {
        spdlog::error("Lua error calling {}: {}", func, e.what());
        return sol::make_object(lua_, sol::nil);
      }
    }
    void RegisterEngineAPI();
    sol::state& GetState() { return lua_;}

  private:
    sol::state lua_;
    bool initialized_ = false;
  };

}
