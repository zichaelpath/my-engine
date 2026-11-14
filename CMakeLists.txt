cmake_minimum_required(VERSION 3.28)
project(MyEngine LANGUAGES CXX)

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

add_executable(MyEngine src/main.cpp)

# NOTE: main.cpp doesn't use ImGui until Lesson 3. The ImGui-SFML link
# below is the Lesson 3 setup, already in place so the build is ready
# to configure when the lesson starts.
target_link_libraries(MyEngine PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
    ImGui-SFML::ImGui-SFML)
