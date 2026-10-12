#include "AudioEngine.h"

AudioEngine::AudioEngine()
{
    const auto result = Pa_Initialize();
    if (result != paNoError)
    {
        fail(result, "Pa_Initialize");
        return;
    }

    portAudioInitialized_ = true;
    start();
}

AudioEngine::~AudioEngine()
{
    shutdown();
}

bool AudioEngine::start()
{
    if (!portAudioInitialized_)
        return false;

    if (stream_ != nullptr)
        return true;

    const auto deviceIndex = Pa_GetDefaultInputDevice();
    if (deviceIndex == paNoDevice)
    {
        lastError_ = "No default audio input device";
        state_.store(State::error);
        return false;
    }

    const auto* deviceInfo = Pa_GetDeviceInfo(deviceIndex);
    PaStreamParameters inputParameters {};
    inputParameters.device = deviceIndex;
    inputParameters.channelCount = 2;
    inputParameters.sampleFormat = paFloat32;
    inputParameters.suggestedLatency = deviceInfo->defaultLowInputLatency;

    auto result = Pa_OpenStream(&stream_,
                                &inputParameters,
                                nullptr,
                                settings_.sampleRate,
                                settings_.bufferSize,
                                paNoFlag,
                                &AudioEngine::audioCallback,
                                this);
    if (result != paNoError)
    {
        stream_ = nullptr;
        return fail(result, "Pa_OpenStream");
    }

    result = Pa_StartStream(stream_);
    if (result != paNoError)
    {
        Pa_CloseStream(stream_);
        stream_ = nullptr;
        return fail(result, "Pa_StartStream");
    }

    lastError_.clear();
    state_.store(State::running);
    return true;
}

void AudioEngine::stop()
{
    if (stream_ != nullptr)
    {
        if (Pa_IsStreamActive(stream_) == 1)
            Pa_StopStream(stream_);

        Pa_CloseStream(stream_);
        stream_ = nullptr;
    }

    state_.store(State::stopped);
}

bool AudioEngine::applySettings(Settings settings)
{
    const bool wasRunning = state() == State::running;
    stop();
    settings_ = settings;
    return !wasRunning || start();
}

void AudioEngine::shutdown()
{
    stop();

    if (portAudioInitialized_)
    {
        Pa_Terminate();
        portAudioInitialized_ = false;
    }
}

AudioEngine::State AudioEngine::state() const noexcept
{
    return state_.load();
}

AudioEngine::Settings AudioEngine::settings() const noexcept
{
    return settings_;
}

const std::string& AudioEngine::lastError() const noexcept
{
    return lastError_;
}

int AudioEngine::audioCallback(const void* input,
                               void*,
                               unsigned long frameCount,
                               const PaStreamCallbackTimeInfo*,
                               PaStreamCallbackFlags,
                               void* userData)
{
    auto& engine = *static_cast<AudioEngine*>(userData);
    engine.process(static_cast<const float*>(input), frameCount);
    return paContinue;
}

void AudioEngine::process(const float* input, unsigned long frameCount) noexcept
{
    // Real-time audio thread: add DSP/onset detection here.
    // Do not allocate, lock, log, or access UI state in this function.
    (void) input;
    (void) frameCount;
}

bool AudioEngine::fail(int errorCode, const char* operation)
{
    lastError_ = std::string(operation) + ": " + Pa_GetErrorText(errorCode);
    state_.store(State::error);
    return false;
}

