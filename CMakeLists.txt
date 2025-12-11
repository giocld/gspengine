cmake_minimum_required(VERSION 3.14...3.31)
project(
    gspengine
    LANGUAGES CXX C
)

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

#declare the engine library
add_library( gspengine STATIC
    src/Engine.cpp
    src/GraphicsManager.cpp
)

set_target_properties( gspengine PROPERTIES CXX_STANDARD 20 )

#declare our engine's header path
target_include_directories( gspengine PUBLIC src )

#link dependencies to the engine
target_link_libraries( gspengine 
    PUBLIC glfw 
    PRIVATE spdlog::spdlog 
)

#define the game executable
add_executable( helloworld demo/helloworld.cpp )

set_target_properties( helloworld PROPERTIES CXX_STANDARD 20 )
add_custom_target( run_helloworld helloworld USES_TERMINAL )

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
