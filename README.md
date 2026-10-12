# Audio app template

Baseline C++ minimale pour une application audio avec :

- PortAudio (callback temps réel / entrée mono) ;
- SDL2 + OpenGL 3.3 ;
- Dear ImGui, branche `docking` ;
- CMake + dépendances téléchargées et épinglées automatiquement.

## Compiler

Prérequis : CMake 3.24+, Git et un compilateur C++17. CMake sélectionne le
générateur disponible sur la machine (Visual Studio, Makefiles, Ninja, etc.).

```bash
cmake --preset debug
cmake --build --preset debug
```

L'exécutable est généré sous `build/debug`. Avec un générateur multi-config
comme Visual Studio, il se trouve dans `build/debug/Debug/audio_app.exe`.

Sous Linux, les paquets de développement OpenGL et ALSA peuvent être nécessaires,
par exemple `libgl1-mesa-dev` et `libasound2-dev` sur une distribution Debian/Ubuntu.

## Structure

```text
src/main.cpp          Cycle de vie et main loop uniquement
src/Application.*     Fenêtre SDL2, contexte OpenGL, ImGui et UI
src/AudioEngine.*     Cycle de vie PortAudio et callback audio
```

Le moteur démarre à sa construction avec l'entrée audio par défaut. Son callback
`AudioEngine::process()` s'exécute sur le thread temps réel créé par PortAudio :
c'est le point d'entrée prévu pour le DSP ou la détection d'onsets.

Les réglages de sample rate et de buffer sont appliqués en redémarrant proprement
le stream. L'UI permet également de démarrer et arrêter le moteur.

