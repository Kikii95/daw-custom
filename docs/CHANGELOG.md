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

### Added — Project Persistence (Sprint 17)
- **ProjectSerializer**: JSON-based project save/load system
  - .dawc file format with version tracking
  - Full project state: tempo, sample rate, master volume
  - Complete track serialization: volume, pan, mute, solo, colour
  - Clip data with source file paths and gain
  - Effect chain persistence with all parameters
  - VST3 plugin state saved as base64-encoded binary
- **File menu integration**: Save/Save As/Open with unsaved changes prompt
- **Project.h clear()**: Reset method for new/load operations

### Added — Preset System (Sprint 18)
- **Preset data structure**: JSON-based preset format with versioning
  - Effect parameters stored as key-value pairs
  - VST3 state support (base64 binary)
  - Metadata: name, author, category, effect type
- **PresetManager**: Central preset management
  - Preset directory structure: ~/Music/DAWCustom/Presets/Effects/<type>/
  - Factory preset creation on first run (15+ presets)
  - Load/save/scan operations
  - applyPreset() and createFromEffect() methods
- **PresetBrowser UI**: CallOutBox popup with ListBox
  - Category filtering via ComboBox
  - Double-click to apply preset
  - Save button for creating new presets
- **EffectSlotComponent**: Added "P" preset button
- **Factory presets included**:
  - EQ: Warm Vocal, Hi-Fi Boost, Low Cut
  - Compressor: Gentle Glue, Punchy Drums, Vocal Control
  - Reverb: Small Room, Large Hall, Plate
  - Delay: Slapback, Stereo Eighth, Ambient
  - KickDesigner: 808 Classic, Punchy House, Deep Sub
  - BasicSynth: Warm Pad, Pluck Lead, Sub Bass

### Fixed — UX Audit (Sprint 15)
- **Audio playback race condition**: Fixed initialization order in AudioEngine — sample rate now obtained before audio callback setup
- **Track selection Timeline**: Added mouseDown() to TrackLane with selection highlight
- **Timeline↔Mixer sync**: Track selection in timeline now syncs with mixer and effect rack

### Added — Asset Browser (Sprint 16)
- **AssetBrowser component**: Unity-style file browser on left panel
- **File tree**: DirectoryContentsList with audio file filtering
- **Recent files**: ListBox with last 10 imported files
- **Folder bookmarks**: Quick access to Music, Downloads, Desktop, Home
- **Double-click import**: Files imported directly on double-click
- **Drag support**: Files can be dragged from browser

### Added — UX Improvements (Sprint 15)
- **Auto-select first track**: First track auto-selected on startup, effect rack populated
- **Clip tooltips**: Hover clips shows name + duration (via SettableTooltipClient)
- **Clip hover effect**: Subtle glow on clip hover
- **Effect callbacks wired**: onEffectAdded/Removed/Bypass/Parameter connected in MainComponent

### Fixed — Core Functionality (Sprint 14)
- Audio playback: Buffer now properly loaded into AudioClip and added to AudioTrack
- Track renaming: Double-click on track header to edit name inline
- Timeline ruler: Aligned with clip start position (headerWidth offset)

### Added — Project Infrastructure (Sprint 14)
- Unit test framework: JUCE UnitTest with AudioTests.cpp
- CMakeLists.txt: BUILD_TESTS option with DAWCustomTests target
- Pre-commit hooks: trailing-whitespace, clang-format, cmake-lint
- Documentation: architecture.md, quickstart.md
- Compliance: LICENSE (GPL-3.0), CONTRIBUTING.md, SECURITY.md
- Audit score: 6/18 → 16/18 (Grade S Gold)

### Added — UI/UX Pro-Level Overhaul (Sprint 13)

**Theme System Enhancement**
- AppTheme.h: +Shadows (sm, md, lg), +Gradients, +TrackColours (12-color palette), +Glow
- DrawingHelpers.h/cpp: Reusable shadow, glow, gradient, beveled button rendering

**ModernLookAndFeel Enhancement**
- Buttons: Beveled 3D effect with gradient, shadow, top highlight, hover glow
- Rotary sliders: Multi-layer glow, gradient knob center
- ComboBox/PopupMenu: Shadows and gradients

**Mixer & Meters (FL Studio style)**
- MeterComponent: LED-style 24-segment meters with gradient lit segments
- Peak hold indicator with glow effect
- ChannelStrip: Drop shadows, gradient backgrounds, color bar with glow, selection glow

**Waveforms & Timeline**
- WaveformDisplay: Gradient waveform rendering with per-track colors
- Path-based rendering with top highlight
- ClipComponent: Drop shadow, gradient background, darker header band, selection glow

**Transport & Effects**
- TransportBar: Inner shadow, time display glow, accent bottom line
- EffectSlotComponent: Drop shadow, header gradient, bypass/active glow indicators

**Modals**
- ModalOverlay: Animated backdrop with vignette, shadow, glow effects
- FirstRunDialog: Gradient background, accent bar glow, top highlight

**CI/CD**
- GitHub Actions workflow: Linux + Windows builds
- Artifact upload for release binaries

### Added — Phase 2 Sprint 6 (Effects Core)

**DSP Architecture**
- EffectSlot: Base class for all effects (ProcessorBase interface)
- EffectChain: Container managing multiple effects in series
- Per-track effect chain integration in AudioTrack

**Effects**
- GainEffect: Volume control + constant power stereo panning
- ParametricEQ: 3-band EQ (Low Shelf, Mid Peak, High Shelf)
- CompressorEffect: Dynamics processor (threshold, ratio, attack, release, makeup)
- ReverbEffect: Freeverb wrapper with room presets (Small Room, Large Hall, Plate)
- DelayEffect: Stereo delay with feedback, ported from legacy XAudio2 code

**UI - Effects (Sprint 7)**
- EffectSlotComponent: Dynamic parameter controls (knobs), bypass toggle, remove button
- EffectRackPanel: Container with effect type combo, add/remove effects, scrollable list
- Track selection in MixerPanel with visual feedback
- Integration with MainComponent layout

**Synthesis - Sprint 8 (Phase 3)**
- Oscillators.h: Waveform generation helpers (sine, square, sawtooth, triangle, noise)
- OscillatorEffect: Standalone oscillator with frequency, waveform type, level params
- EnvelopeEffect: ADSR envelope with trigger/release, auto-trigger mode
- Integration with EffectRackPanel (Oscillator + Envelope in combo menu)

**Kick Designer - Sprint 9 (Phase 3)**
- KickDesigner: 808-style kick drum synthesizer
  - Sine oscillator with pitch envelope (exponential decay)
  - Noise layer with separate decay for attack transient
  - Click component for punch
  - Saturation (soft clipping via tanh)
  - 9 parameters: Freq, Pitch, P.Decay, Decay, Noise, N.Decay, Drive, Click, Level

**Basic Synth + Filter - Sprint 10 (Phase 3)**
- SimpleFilter: Resonant LP/HP/BP filter using StateVariableTPTFilter
  - Cutoff, Resonance, Mode, EnvAmount parameters
  - Envelope modulation input for external control
- BasicSynth: Subtractive synthesizer
  - Dual oscillators with detune (5 waveforms)
  - Resonant lowpass filter with envelope modulation
  - Full ADSR amplitude envelope
  - 12 parameters: Freq, Wave, Detune, Osc2, Cutoff, Reso, FltEnv, Atk, Dec, Sus, Rel, Level

**Plugin Infrastructure - Sprint 11 (Phase 4)**
- PluginManager: Singleton for VST3 plugin discovery and management
  - Background async scanning with progress callback
  - Platform-specific search paths (Linux, macOS, Windows)
  - Plugin list caching (XML persistence)
  - Deadman's pedal for crash detection during scanning
  - Async plugin instantiation (required for stability)
- VST3EffectSlot: Wrapper adapting AudioPluginInstance to EffectSlot interface
  - Maps plugin parameters to EffectSlot interface
  - Support for plugin editor (UI)
  - State save/load for presets
  - Latency reporting for host compensation

**Plugin UI - Sprint 12 (Phase 4)**
- PluginEditorWindow: DocumentWindow hosting native VST3 plugin UIs
  - Window management (open, close, bring to front)
  - Automatic cleanup on effect removal
  - Static factory methods for window tracking
- GenericPluginEditor: Fallback parameter grid for plugins without native UI
  - Dynamic grid of rotary sliders
  - Timer-based parameter polling
  - Automatic layout based on parameter count
- EffectRackPanel VST3 integration:
  - "Scan" button for background VST3 scanning
  - VST3 plugins section in effect combo menu
  - Async plugin loading with progress feedback
- MainComponent integration:
  - PluginManager initialization at startup
  - Plugin window cleanup on shutdown

### Planned
- Phase 4 complete ✅
- Sprint 17-18 complete ✅ (Save/Load + Presets)
- Next: UI polish, Undo/Redo, MIDI support
