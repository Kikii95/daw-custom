# Quick Start Guide

Get DAW Custom running in 5 minutes.

## Prerequisites

- **CMake** 3.22+
- **C++20 compiler**: GCC 11+, Clang 14+, or MSVC 2022
- **Linux**: ALSA dev libraries (`libasound2-dev`)
- **Linux**: X11/Wayland dev libraries

### Ubuntu/Debian

```bash
sudo apt update
sudo apt install build-essential cmake git
sudo apt install libasound2-dev libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libfreetype6-dev libwebkit2gtk-4.0-dev
```

### Windows

Install Visual Studio 2022 with C++ workload, and CMake.

## Build

```bash
# Clone
git clone https://github.com/Kikii95/daw-custom.git
cd daw-custom

# Configure (first build downloads JUCE ~2min)
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build -j$(nproc)
```

## Run

```bash
# Linux
./build/DAWCustom_artefacts/Release/DAW\ Custom

# Windows
.\build\DAWCustom_artefacts\Release\DAW Custom.exe
```

## First Steps

### 1. Import Audio

- **Drag & drop** an audio file (WAV, MP3, FLAC) onto the timeline
- Or use **File > Import Audio**

### 2. Playback

- Click **Play** button or press `Space`
- Use transport bar to seek

### 3. Add Tracks

- **Edit > Add Track** to create new tracks
- Each track has volume, pan, mute, solo controls

### 4. Add Effects

- Select a track in the mixer
- Use the effect rack (right panel) to add effects:
  - Gain, EQ, Compressor, Reverb, Delay
  - Kick Designer, Basic Synth
  - VST3 plugins (scan first)

### 5. Export

- **File > Export Mix** to render to WAV or FLAC

## Keyboard Shortcuts

| Key | Action |
|-----|--------|
| `Space` | Play/Pause |
| `Ctrl+Z` | Undo (planned) |
| `Ctrl+S` | Save (planned) |
| `Ctrl+Wheel` | Zoom timeline |

## Troubleshooting

### No audio output

1. Check audio device in system settings
2. Ensure no other app is using exclusive audio

### VST3 plugins not found

1. Use the "Scan" button in effect rack
2. Plugins are searched in standard paths:
   - Linux: `~/.vst3`, `/usr/lib/vst3`
   - Windows: `C:\Program Files\Common Files\VST3`

## Next Steps

- Read [architecture.md](architecture.md) for project structure
- Check [BACKLOG.md](BACKLOG.md) for planned features
- See [CONTRIBUTING.md](../CONTRIBUTING.md) to contribute
