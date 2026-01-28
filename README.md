# DAW Custom

Personal DAW for mashups and sound design.

## Stack

- **Framework**: JUCE 7 (GPL)
- **Language**: C++20
- **Build**: CMake

## Features (Planned)

- Multi-track mixer
- Audio import/export (WAV, MP3, FLAC)
- Built-in effects (EQ, Compressor, Reverb, Delay)
- Kick designer
- Basic synthesizer
- VST3 plugin hosting

## Build

```bash
# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build --config Release

# Run
./build/DAWCustom_artefacts/Release/DAWCustom
```

## Requirements

- CMake 3.22+
- C++20 compiler (GCC 11+, Clang 14+, MSVC 2022)
- Linux: ALSA dev libraries (`libasound2-dev`)

### Linux Dependencies

```bash
sudo apt-get install libasound2-dev libcurl4-openssl-dev libfreetype6-dev \
    libx11-dev libxcomposite-dev libxcursor-dev libxinerama-dev libxrandr-dev \
    libxrender-dev libwebkit2gtk-4.0-dev libglu1-mesa-dev mesa-common-dev
```

## License

GPL v3 (due to JUCE GPL license)
