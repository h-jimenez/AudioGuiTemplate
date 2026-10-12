cmake_minimum_required(VERSION 3.24)

project(AudioAppTemplate VERSION 0.1.0 LANGUAGES C CXX)

# PortAudio 19.7.0 still declares compatibility with very old CMake releases.
# CMake 4.x needs this floor to configure that pinned dependency.
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

# Keep every dependency pinned so a fresh checkout produces the same build.
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_TEST OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG        5d249570393f7a37e037abf22cd6012a4cc56a71 # release-2.32.10
    GIT_SHALLOW    TRUE
)

set(PA_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(PA_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(PA_BUILD_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    portaudio
    GIT_REPOSITORY https://github.com/PortAudio/portaudio.git
    GIT_TAG        147dd722548358763a8b649b3e4b41dfffbcfbb6 # v19.7.0
    GIT_SHALLOW    TRUE
)

# This commit is the v1.92.9b-docking tag from Dear ImGui's docking branch.
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        9acdfbf46810c0c74ab281ce04122c4149ae8bd1
    GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(SDL2 portaudio imgui)

find_package(OpenGL REQUIRED)

# PortAudio 19.7.0 predates the namespaced CMake target convention.
add_library(PortAudio::PortAudio ALIAS portaudio_static)

add_library(imgui_backend STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl2.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui_backend PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

target_link_libraries(imgui_backend PUBLIC
    SDL2::SDL2-static
    OpenGL::GL
)

add_executable(audio_app
    src/main.cpp
    src/Application.cpp
    src/AudioEngine.cpp
)

target_include_directories(audio_app PRIVATE src)
target_compile_definitions(audio_app PRIVATE SDL_MAIN_HANDLED)
target_link_libraries(audio_app PRIVATE
    imgui_backend
    PortAudio::PortAudio
)

if(MSVC)
    target_compile_options(audio_app PRIVATE /W4 /permissive-)
else()
    target_compile_options(audio_app PRIVATE -Wall -Wextra -Wpedantic)
endif()

