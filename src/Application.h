#pragma once

#include "AudioEngine.h"

struct SDL_Window;
using SDL_GLContext = void*;

class Application
{
public:
    Application();
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    [[nodiscard]] bool isRunning() const noexcept;
    void render(AudioEngine& audioEngine);
    void shutdown();

private:
    void drawAudioSettings(AudioEngine& audioEngine);

    SDL_Window* window_ = nullptr;
    SDL_GLContext glContext_ = nullptr;
    bool running_ = true;
    bool initialized_ = false;
    int selectedSampleRate_ = 48000;
    int selectedBufferSize_ = 256;
};

