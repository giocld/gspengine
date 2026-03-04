cmake_minimum_required(VERSION 3.14...3.31)
project(
    gspengine LANGUAGES CXX C)

#enable decentralized package loading
include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)

#fetch glfw for window creation
FetchContent_Declare(
    glfw3
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable( glfw3 )

#fetch spdlog for logging
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog/
    GIT_TAG v1.15.3
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE

)
FetchContent_MakeAvailable( spdlog )

#fetch soloud
FetchContent_Declare(
  soloud
  GIT_REPOSITORY https://github.com/jarikomppa/soloud
  GIT_TAG        e82fd32c1f62183922f08c14c814a02b58db1873
  GIT_SHALLOW FALSE
  GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable( soloud )

#soloud manual library definition
file(GLOB soloud_sources
    "${soloud_SOURCE_DIR}/src/audiosource/*/*.c*"
    "${soloud_SOURCE_DIR}/src/c_api/*.c*"
    "${soloud_SOURCE_DIR}/src/core/*.c*"
    "${soloud_SOURCE_DIR}/src/filter/*.c*"
    "${soloud_SOURCE_DIR}/src/backend/miniaudio/soloud_miniaudio.cpp"
)

add_library(soloud ${soloud_sources})
target_compile_definitions(soloud PRIVATE WITH_MINIAUDIO)
target_include_directories(soloud PUBLIC "${soloud_SOURCE_DIR}/include")

#fetch glad for OpenGL loading
FetchContent_Declare(
  glad
  GIT_REPOSITORY https://github.com/Dav1dde/glad.git
  GIT_TAG        v2.0.8
  GIT_SHALLOW TRUE
  GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable( glad )

# Generate glad sources
add_subdirectory("${glad_SOURCE_DIR}/cmake" glad_cmake)
glad_add_library(glad_gl_core_33 REPRODUCIBLE API gl:core=3.3)

#fetch glm math library
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 2d4c4b4dd31fde06cfffad7915c2b3006402322f
    GIT_SHALLOW FALSE
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable( glm )
#load sprites into cpu
FetchContent_Declare(
    stb_image
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG master
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)
#fetch lua
FetchContent_Declare(
  lua
  GIT_REPOSITORY https://github.com/lua/lua.git
  GIT_TAG v5.4.7
)
FetchContent_MakeAvailable(lua)

#fetch sol2
FetchContent_Declare(
  sol2
  GIT_REPOSITORY https://github.com/ThePhD/sol2.git
  GIT_TAG develop
  GIT_SHALLOW FALSE
)
FetchContent_MakeAvailable(sol2)

FetchContent_MakeAvailable( stb_image )
add_library( stb INTERFACE )
target_include_directories( stb INTERFACE ${stb_image_SOURCE_DIR} )
# Find OpenGL
find_package(OpenGL REQUIRED)

if(APPLE)
    find_library(AudioUnit_LIBRARY AudioUnit)
    find_library(CoreAudio_LIBRARY CoreAudio)
    target_link_libraries(soloud INTERFACE ${AudioUnit_LIBRARY} ${CoreAudio_LIBRARY})
    target_compile_definitions(soloud PRIVATE MA_NO_RUNTIME_LINKING)
endif()

if(WIN32)
    target_compile_definitions( soloud PRIVATE MA_NO_JACK )
endif()

#declare the engine library
add_library( gspengine STATIC
    src/Engine.cpp
    src/Engine.h
    src/GraphicsManager.cpp
    src/GraphicsManager.h
    src/InputManager.cpp
    src/InputManager.h
    src/ResourceManager.cpp
    src/ResourceManager.h
    src/SoundManager.cpp
    src/SoundManager.h
    src/ScriptManager.cpp
    src/ScriptManager.h
    src/Types.h
    # ECS files
    src/ECS/Entity.h
    src/ECS/Component.h
    src/ECS/PositionComponent.h
    src/ECS/SpriteComponent.h
    src/ECS/VelocityComponent.h
    src/ECS/Registry.h
)

set_target_properties( gspengine PROPERTIES CXX_STANDARD 20 )

#declare our engine's header path
#added soloud include path here so the engine can find soloud.h
target_include_directories( gspengine PUBLIC
    src
    "${soloud_SOURCE_DIR}/include"
)

#link dependencies to the engine
target_link_libraries( gspengine
    PUBLIC glfw
    PUBLIC soloud
    PUBLIC spdlog::spdlog
    PUBLIC glad_gl_core_33
    PUBLIC OpenGL::GL
    PUBLIC glm
    PUBLIC stb
    PUBLIC lua
    PUBLIC sol2
)

#define the game executable
add_executable( helloworld demo/helloworld.cpp )

set_target_properties( helloworld PROPERTIES CXX_STANDARD 20 )
add_custom_target( run_helloworld
  COMMAND helloworld
  USES_TERMINAL
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

#link the engine library to the demo executable
target_link_libraries( helloworld PRIVATE gspengine )

#sanitizers (optional)
option( USE_SANITIZER "Use Address Sanitizer" ON )
if( USE_SANITIZER )
    if(MSVC)
        add_compile_options(/fsanitize=address)
        add_link_options(/fsanitize=address)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        add_compile_options(-fsanitize=address)
        add_link_options(-fsanitize=address)
    endif()
endif()

# ECS Registry Test
add_executable( registrytest src/ECS/RegistryTest.cpp )
set_target_properties( registrytest PROPERTIES CXX_STANDARD 20 )
target_link_libraries( registrytest PRIVATE gspengine )
