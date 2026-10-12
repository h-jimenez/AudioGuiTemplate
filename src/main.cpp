#include "Application.h"
#include "AudioEngine.h"

#include <exception>
#include <iostream>

int main()
{
    try
    {
        Application app;
        AudioEngine audioEngine;

        while (app.isRunning())
            app.render(audioEngine);

        audioEngine.shutdown();
        app.shutdown();
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }

    return 0;
}

