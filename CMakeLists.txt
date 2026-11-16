cmake_minimum_required(VERSION 3.28)
project(MyEngine VERSION 1.5.0 LANGUAGES C CXX)

# The engine's version lives in the project() line above and
# nowhere else: configure_file bakes it into a generated header the
# hosts include, so every binary reports the same version and a
# release bump is a one-line change.
configure_file(src/Version.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/generated/Version.hpp @ONLY)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# --- SFML 3.0.2 ---
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.2)
FetchContent_MakeAvailable(SFML)

# --- Dear ImGui v1.91.8, DOCKING branch ---
# Sources only: ImGui ships no CMakeLists.txt of its own, so this just
# downloads the code. ImGui-SFML below is what actually builds it.
# The -docking tag is the same 1.91.8 release from ImGui's docking
# branch — docking (DockSpace, snap-together panels) doesn't exist in
# the master-branch build. Switched in Lesson 6.
FetchContent_Declare(imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.8-docking)
FetchContent_MakeAvailable(imgui)

# --- ImGui-SFML v3.0 ---
# The glue library that lets ImGui draw inside an SFML window.
# IMGUI_DIR points it at the ImGui sources we just fetched, and
# IMGUI_SFML_FIND_SFML OFF tells it to use the already-fetched SFML
# above instead of looking for a system install.
set(IMGUI_DIR ${imgui_SOURCE_DIR} CACHE PATH "Path to Dear ImGui sources")
set(IMGUI_SFML_FIND_SFML OFF CACHE BOOL "Use the FetchContent SFML")
FetchContent_Declare(imgui-sfml
    GIT_REPOSITORY https://github.com/SFML/imgui-sfml.git
    GIT_TAG v3.0)
FetchContent_MakeAvailable(imgui-sfml)

# --- Lua 5.4.7 (Lesson 66, Arc 5) ---
# The scripting language, embedded from the official lua.org
# tarball (hash pinned). Lua ships no CMakeLists, so the library
# is built here from its C sources — everything in src/ except
# the two standalone programs (lua.c the interpreter, luac.c the
# compiler). Hand-bound through the plain C API, like the
# engine's GL loader: small enough to own, nothing hidden.
FetchContent_Declare(lua
    URL https://www.lua.org/ftp/lua-5.4.7.tar.gz
    URL_HASH SHA256=9fbf5e28ef86c69858f6d3d34eccc32e911c1a28b4120ff3e84aaa70cfbf1e30)
FetchContent_GetProperties(lua)
if(NOT lua_POPULATED)
    FetchContent_Populate(lua)
endif()
file(GLOB LUA_SOURCES ${lua_SOURCE_DIR}/src/*.c)
list(REMOVE_ITEM LUA_SOURCES
    ${lua_SOURCE_DIR}/src/lua.c
    ${lua_SOURCE_DIR}/src/luac.c)
add_library(lua STATIC ${LUA_SOURCES})
target_include_directories(lua PUBLIC ${lua_SOURCE_DIR}/src)
if(UNIX)
    target_link_libraries(lua PUBLIC m dl)
endif()

add_executable(MyEngine src/main.cpp)

# NOTE: main.cpp doesn't use ImGui until Lesson 3. The ImGui-SFML link
# below is the Lesson 3 setup, already in place so the build is ready
# to configure when the lesson starts.
target_link_libraries(MyEngine PRIVATE
    GameLib
    ImGui-SFML::ImGui-SFML)

add_executable(MyRuntime src/runtime_main.cpp)

target_link_libraries(MyRuntime PRIVATE
    GameLib)

add_library(GameLib src/Game.cpp src/Random.cpp src/Noise.cpp src/Log.cpp src/Config.cpp src/SpriteSheet.cpp src/Resources.cpp src/Project.cpp src/Particles.cpp src/ScriptHost.cpp src/Campaign.cpp src/Ui.cpp src/SaveData.cpp)

target_include_directories(GameLib PUBLIC src ${CMAKE_CURRENT_BINARY_DIR}/generated)

target_link_libraries(GameLib PUBLIC
    SFML::Graphics
    SFML::Window
    SFML::System
    SFML::Audio
    lua)

# --- OpenGL (Lesson 35, Arc 8) ---
# GLLoader is the engine's own function loader: post-1.1 entry points
# resolved at runtime from the driver (see src/GLLoader.hpp). It uses
# Log from GameLib. OpenGL::GL carries both the platform library
# (libGL / opengl32) and — the part that bit in Lesson 38 — the
# include path for <GL/gl.h>, wherever this machine keeps it.
find_package(OpenGL REQUIRED)

add_library(GLLoader src/GLLoader.cpp)

target_include_directories(GLLoader PUBLIC src vendor)

target_link_libraries(GLLoader PUBLIC GameLib OpenGL::GL)

# MyGL: the raw-OpenGL host. SFML provides the window/context/events
# only (sf::Window, not RenderWindow — no SFML rendering state).
add_executable(MyGL src/gl_main.cpp)

target_link_libraries(MyGL PRIVATE
    GLLoader
    SFML::Window
    SFML::System)

# The headless test harness (Lesson 45): the Phase 1 probes, grown
# up into one command — determinism, scene round-trips, project and
# config parsing, and the Lesson 44 flip regression. Exit code is
# the number of failures, so CI can read it.
add_executable(MyTests tests/tests_main.cpp)

target_link_libraries(MyTests PRIVATE GameLib)

