# Changelog — DAW Custom

All notable changes to this project will be documented in this file.

Format based on [Keep a Changelog](https://keepachangelog.com/).

---

## [Unreleased] - Sprint H: Pro Editing Workflow

### Added

**Fades & Crossfades** (Phase 1)
- Fade In/Out handles on clip edges — draggable with visual triangle overlay
- 4 fade curve types: Linear, Exponential, S-Curve, Logarithmic
- DSP fade processing in `AudioClip::getNextAudioBlock()` with gain curves
- Automatic crossfade detection — overlapping clips get auto-crossfades (up to 2s)
- `FadeClipCommand` for undo/redo support
- Fade data persisted in .dawc project files

**Markers System** (Phase 2)
- `Marker` model with id, name, time, colour, shortcut number (1-9)
- Visual markers on TimelineRuler — flag shape with vertical line
- `Shift+M` — Add marker at playhead position
- `Numpad 1-9` — Jump to marker by shortcut number
- Click marker to jump, right-click to delete
- Markers serialized in .dawc project files

**Zoom Enhancements** (Phase 3)
- `Ctrl+0` — Zoom to fit all clips
- `Ctrl+1` — Zoom to selection (10% padding)
- `zoomToTimeRange(start, end)` helper for programmatic zoom

**Clip Grouping** (Phase 4)
- `ClipGroup` model — groups clips across tracks
- `Ctrl+G` — Group selected clips
- `Ctrl+Shift+G` — Ungroup selected clips
- Groups persisted in .dawc project files
- Group management in `Project`: createGroup, dissolveGroup, getGroupForClip

**Ripple Edit Mode** (Phase 5)
- `EditMode` enum: Normal vs Ripple
- `R` key — Toggle ripple edit mode
- Deleting clips in ripple mode shifts subsequent clips to fill the gap
- Full undo support for ripple operations

### Changed
- `Track::getClip(uuid)` method added for direct clip lookup
- `AudioClip::setStartTime()` method for ripple edit operations
- `DeleteClipsCommand` now handles ripple shifts with proper undo

---

## [Unreleased] - Sprint G: Undo/Redo & Advanced Editing

### Added
- **Undo/Redo System** — Full Command Pattern implementation
  - `UndoableCommand` base class with 500ms merge window for continuous drags
  - `UndoManager` with dual-stack (undo/redo), max 100 actions
  - `MoveClipCommand` — Move clips with multi-selection support
  - `DeleteClipsCommand` — Delete with full data restoration on undo
  - `AddClipCommand` — For duplicate, paste, import operations
  - `TrimClipCommand` — Trim clip edges with merge support
  - `MoveClipToTrackCommand` — Move clips between tracks

- **Keyboard Shortcuts**
  - `Ctrl+Z` — Undo
  - `Ctrl+Y` / `Ctrl+Shift+Z` — Redo
  - `Ctrl+D` — Duplicate selected clips (multi-selection)
  - `Ctrl+Shift+A` — Select all clips globally (all tracks)

- **Snap to Clip Edge** — Snaps to start/end of other clips (100ms threshold)

### Changed
- `snapToGrid()` now checks both grid lines AND clip edges
- Clip operations (delete, duplicate, move) now use undo system

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

### Added — Drag & Drop Workflow Improvement (Sprint F)

**Clip Drag Freedom**
- Drag clips directly to any track (not just adjacent tracks)
- Absolute track index system replaces incremental delta
- Query-based track detection via `onQueryTrackAtY` callback

**Browser to Timeline**
- Drag audio files from AssetBrowser directly to Timeline
- DragAndDropTarget interface in TimelinePanel
- Drop position determines track + start time
- Visual feedback (track highlight) during drag

**Smart Import**
- Double-click in browser imports to selected track (not Track 1)
- Import at playhead position
- `importFileToTrack()` centralized helper method

**Visual Feedback**
- Consistent drop target highlighting across all drag operations
- Clear drop targets on drag end

---

### Added — Advanced Editing & BPM Workflow (Sprint E)

**Clip Editing**
- Split clip at playhead (Ctrl+B) or at click position (Alt+Click)
- Trim handles on clip edges: drag left/right edges to adjust duration
- Paste at last click position (5s timeout) instead of playhead only
- Copy/Paste clips maintains relative timing

**BPM & Beat Grid**
- BPM synchronization between Project ↔ TransportController ↔ Timeline
- Beat grid on ruler: bar lines (thick) + beat lines (thin) with bar numbers
- Beat-based snap: 1/Bar, 1/Beat, 1/2, 1/4, 1/8, 1/16 note values
- Snap selector dropdown in TransportBar
- Position display toggle: MM:SS.ms ↔ Bar:Beat:Tick (960 PPQ)

**Delay Tempo Sync**
- Sync button + note value selector (1/2, 1/4, 1/8, 1/16, dotted) in Delay effect
- Auto-calculate delay time from BPM and note value
- Tempo propagation to EffectRackPanel → EffectSlotComponent

**UI/UX**
- Trim cursor (↔) when hovering clip edges
- Visual feedback on trim zones
- Click position tracking for paste operations

---

### Added — UX Overhaul (Sprint 19)
- **Transport icons**: Path drawing (triangle, bars, square) instead of UTF-8 characters
- **Drop to specific track**: Audio files drop on the track under cursor
- **Visual drop feedback**: Track highlight during drag operation
- **Clip inter-track dragging**: Vertical drag moves clips between tracks
- **8 default tracks**: New projects start with 8 color-coded tracks
- **Context menus**:
  - Clips: Delete, Duplicate (right-click)
  - Tracks: Rename, Change Colour, Delete (right-click header)
- **Keyboard shortcuts**:
  - Space: Play/Pause
  - Enter: Stop
  - Home: Go to start
  - M: Mute selected track
  - S: Solo selected track
  - Ctrl+S/O/N/T: Save, Open, New, Add Track
- **Track colour picker**: 12-colour palette in context menu
- **TrackColours::getName()**: Colour names for picker menu

### Fixed — UX Bugs (Sprint 19)
- **Transport button states**: Stop button enabled when position > 0, Play resets position when at end
- **Track rename propagation**: Callback chain from Timeline to Mixer to AudioEngine
- **UTF-8 encoding**: Replaced em-dash `\xe2\x80\x94` with `-` in EffectRack and Mixer

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
