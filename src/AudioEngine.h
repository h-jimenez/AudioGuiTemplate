#pragma once

#include <portaudio.h>

#include <atomic>
#include <string>

class AudioEngine
{
public:
    enum class State
    {
        stopped,
        running,
        error
    };

    struct Settings
    {
        double sampleRate = 48000.0;
        unsigned long bufferSize = 256;
    };

    AudioEngine();
    ~AudioEngine();

    AudioEngine(const AudioEngine&) = delete;
    AudioEngine& operator=(const AudioEngine&) = delete;

    bool start();
    void stop();
    bool applySettings(Settings settings);
    void shutdown();

    [[nodiscard]] State state() const noexcept;
    [[nodiscard]] Settings settings() const noexcept;
    [[nodiscard]] const std::string& lastError() const noexcept;

private:
    static int audioCallback(const void* input,
                             void* output,
                             unsigned long frameCount,
                             const PaStreamCallbackTimeInfo* timeInfo,
                             PaStreamCallbackFlags statusFlags,
                             void* userData);

    void process(const float* input, unsigned long frameCount) noexcept;
    bool fail(int errorCode, const char* operation);

    PaStream* stream_ = nullptr;
    Settings settings_;
    std::atomic<State> state_ { State::stopped };
    std::string lastError_;
    bool portAudioInitialized_ = false;
};

