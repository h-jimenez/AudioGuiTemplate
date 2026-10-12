#include "Application.h"

#include <SDL.h>
#include <SDL_opengl.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl2.h>

#include <stdexcept>
#include <string>

namespace
{
constexpr const char* glslVersion = "#version 330";
}

Application::Application()
{
    SDL_SetMainReady();
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
        throw std::runtime_error(std::string("SDL_Init: ") + SDL_GetError());

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    window_ = SDL_CreateWindow("Audio app template",
                               SDL_WINDOWPOS_CENTERED,
                               SDL_WINDOWPOS_CENTERED,
                               960,
                               600,
                               SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    if (window_ == nullptr)
    {
        const std::string error = SDL_GetError();
        SDL_Quit();
        throw std::runtime_error("SDL_CreateWindow: " + error);
    }

    glContext_ = SDL_GL_CreateContext(window_);
    if (glContext_ == nullptr)
    {
        const std::string error = SDL_GetError();
        SDL_DestroyWindow(window_);
        window_ = nullptr;
        SDL_Quit();
        throw std::runtime_error("SDL_GL_CreateContext: " + error);
    }

    SDL_GL_MakeCurrent(window_, glContext_);
    SDL_GL_SetSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    auto& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui::StyleColorsDark();

    if (!ImGui_ImplSDL2_InitForOpenGL(window_, glContext_)
        || !ImGui_ImplOpenGL3_Init(glslVersion))
    {
        shutdown();
        throw std::runtime_error("Dear ImGui backend initialization failed");
    }

    initialized_ = true;
}

Application::~Application()
{
    shutdown();
}

bool Application::isRunning() const noexcept
{
    return running_;
}

void Application::render(AudioEngine& audioEngine)
{
    SDL_Event event;
    while (SDL_PollEvent(&event) != 0)
    {
        ImGui_ImplSDL2_ProcessEvent(&event);
        if (event.type == SDL_QUIT
            || (event.type == SDL_WINDOWEVENT
                && event.window.event == SDL_WINDOWEVENT_CLOSE
                && event.window.windowID == SDL_GetWindowID(window_)))
        {
            running_ = false;
        }
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());
    drawAudioSettings(audioEngine);

    ImGui::Render();
    int width = 0;
    int height = 0;
    SDL_GL_GetDrawableSize(window_, &width, &height);
    glViewport(0, 0, width, height);
    glClearColor(0.10F, 0.10F, 0.12F, 1.0F);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(window_);
}

void Application::shutdown()
{
    running_ = false;

    if (initialized_)
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
        initialized_ = false;
    }

    if (glContext_ != nullptr)
    {
        SDL_GL_DeleteContext(glContext_);
        glContext_ = nullptr;
    }

    if (window_ != nullptr)
    {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }

    SDL_Quit();
}

void Application::drawAudioSettings(AudioEngine& audioEngine)
{
    static constexpr int sampleRates[] { 44100, 48000, 88200, 96000 };
    static constexpr int bufferSizes[] { 64, 128, 256, 512, 1024 };

    ImGui::Begin("Audio engine");

    const char* stateLabel = "Stopped";
    ImVec4 stateColor { 0.8F, 0.8F, 0.8F, 1.0F };
    if (audioEngine.state() == AudioEngine::State::running)
    {
        stateLabel = "Running";
        stateColor = { 0.3F, 0.9F, 0.4F, 1.0F };
    }
    else if (audioEngine.state() == AudioEngine::State::error)
    {
        stateLabel = "Error";
        stateColor = { 1.0F, 0.35F, 0.3F, 1.0F };
    }

    ImGui::Text("State:");
    ImGui::SameLine();
    ImGui::TextColored(stateColor, "%s", stateLabel);

    if (audioEngine.state() == AudioEngine::State::error)
        ImGui::TextWrapped("%s", audioEngine.lastError().c_str());

    if (ImGui::BeginCombo("Sample rate", std::to_string(selectedSampleRate_).c_str()))
    {
        for (const auto sampleRate : sampleRates)
        {
            const bool selected = sampleRate == selectedSampleRate_;
            if (ImGui::Selectable(std::to_string(sampleRate).c_str(), selected))
                selectedSampleRate_ = sampleRate;
            if (selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    if (ImGui::BeginCombo("Buffer size", std::to_string(selectedBufferSize_).c_str()))
    {
        for (const auto bufferSize : bufferSizes)
        {
            const bool selected = bufferSize == selectedBufferSize_;
            if (ImGui::Selectable(std::to_string(bufferSize).c_str(), selected))
                selectedBufferSize_ = bufferSize;
            if (selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    if (ImGui::Button("Apply"))
    {
        audioEngine.applySettings({ static_cast<double>(selectedSampleRate_),
                                    static_cast<unsigned long>(selectedBufferSize_) });
    }

    ImGui::SameLine();
    if (audioEngine.state() == AudioEngine::State::running)
    {
        if (ImGui::Button("Stop"))
            audioEngine.stop();
    }
    else if (ImGui::Button("Start"))
    {
        audioEngine.start();
    }

    ImGui::End();
}

