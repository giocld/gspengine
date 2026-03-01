#include "ScriptManager.h"
#include "Engine.h"
#include "SoundManager.h"
#include "GraphicsManager.h"
#include "spdlog/spdlog.h"
#include <string>

namespace gspengine {
  ScriptManager::ScriptManager() = default;

  ScriptManager::~ScriptManager(){
    Shutdown();
  }

  bool ScriptManager::Init(){
    lua_.open_libraries(sol::lib::base, sol::lib::math,
        sol::lib::string, sol::lib::table);

    RegisterEngineAPI();

    initialized_ = true;
    spdlog::info("ScriptManager initialized");
    return true;
  }

  void ScriptManager::Shutdown() {
  if (initialized_){
      lua_.collect_garbage();
      initialized_ = false;
      spdlog::info("ScriptManager Shutdown");
    }
  }

  bool ScriptManager::LoadScript(const std::string& path) {
  try{
    lua_.script_file(path);
    spdlog::info("Loaded script: {}", path);
    return true;
    }
  catch(const sol::error& e) {
    spdlog::error("Failed to load script {}: {}",
  path, e.what());
    return false;
    }
  }
  bool ScriptManager::ExecuteString(const std::string& code)
  {
    try {
    lua_.script(code);
    return true;
    } catch (sol::error& e) {
      spdlog::error("Script error: {}", e.what());
      return false;
    }
  }
  void ScriptManager::RegisterEngineAPI(){
    auto engine = lua_["engine"].get_or_create<sol::table>();
    //expose logging
    engine["log"] = lua_.create_table();
    engine["log"]["info"] = [](const std::string& msg) {
  spdlog::info(msg); };
    engine["log"]["error"] = [](const std::string& msg) {
  spdlog::error(msg); };

    engine["dt"] = 0.0f;

    //TODO: Add more bindings as i develop this
  }
}
