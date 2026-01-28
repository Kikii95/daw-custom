# Changelog — DAW Custom

All notable changes to this project will be documented in this file.

Format based on [Keep a Changelog](https://keepachangelog.com/).

---

## [0.1.0] - 2026-01-28

### Added — Phase 1 MVP

**Audio Core**
- AudioEngine: Device management + audio routing via JUCE AudioDeviceManager
- TransportController: Play/pause/stop with position tracking
- AudioClip: PositionableAudioSource for clip playback
- AudioTrack: Multi-clip track with volume/pan
- AudioMixer: Multi-track mixing with master volume, solo/mute

**File I/O**
- AudioFileLoader: Import WAV, MP3, FLAC, OGG
- AudioFileExporter: Export to WAV, FLAC

**UI - Timeline**
- MainLayout: App structure with transport/timeline/mixer areas
- TransportBar: Play/Stop buttons, position display, tempo slider
- TimelineRuler: Time markers + playhead
- TrackLane: Track visualization with clip components
- ClipComponent: Draggable clip with waveform
- WaveformCache + WaveformDisplay: Efficient waveform rendering

**UI - Mixer**
- MixerPanel: Channel strips container + master section
- ChannelStrip: Volume fader, pan knob, mute/solo buttons
- MeterComponent: VU-meter with dB scale, peak hold

**Features**
- Drag & drop audio file import
- Menu bar (File/Edit/View)
- Zoom in/out timeline
- Multi-track playback

**Infrastructure**
- JUCE 7.0.9 via CMake FetchContent
- C++20 standard
- Cross-platform (Linux/macOS/Windows)

### Reference
- Legacy XAudio2 code preserved in `_reference/` for DSP logic reference
- Ported `Tweens.cpp` (30+ easing functions)

---

## [Unreleased]

### Planned
- Phase 2: Effects (EQ, Compressor, Reverb, Delay)
- Phase 3: Sound Design (Kick designer, Synth)
- Phase 4: VST3 support
