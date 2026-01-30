# Backlog — DAW Custom

## ✅ Phase 1 : Mixer + Import (TERMINÉE)

- [x] Setup JUCE + CMake
- [x] Fenêtre principale avec layout basique
- [x] Import audio (WAV, MP3, FLAC)
- [x] Waveform display
- [x] Timeline multi-pistes
- [x] Transport (play, pause, stop)
- [x] Mixer basique (volume, pan, mute/solo)
- [x] Export audio (WAV, FLAC)
- [x] Drag & drop fichiers
- [x] VU-mètres (dB scale)
- [x] Menu bar

## 🎛️ Phase 2 : Effects ✅

### Sprint 6 — Architecture + Effets Core ✅
- [x] Architecture effect chain (EffectSlot, EffectChain)
- [x] GainEffect (volume + constant power pan)
- [x] EQ paramétrique (3 bandes: Low/Mid/High)
- [x] Compressor (threshold, ratio, attack, release, makeup)
- [x] Reverb (Freeverb wrapper)
- [x] Delay (port legacy XAudio2 avec smoothing)
- [x] Effect bypass support
- [x] Intégration AudioTrack

### Sprint 7 — UI ✅
- [x] Effect rack UI panel (EffectRackPanel)
- [x] Effect slot component (EffectSlotComponent: knobs, bypass, remove)
- [x] Track selection in MixerPanel
- [x] Integration MainComponent

## 🎹 Phase 3 : Sound Design ✅

### Sprint 8 — Core Synthesis ✅
- [x] Oscillators.h (waveform helpers: sine, square, saw, triangle, noise)
- [x] OscillatorEffect (frequency, waveform type, level)
- [x] EnvelopeEffect (ADSR with trigger/release, auto-trigger)
- [x] EffectRackPanel integration (combo menu)

### Sprint 9 — Kick Designer ✅
- [x] KickDesigner (sine + pitch envelope + noise + saturation)
- [ ] One-shot export (future)

### Sprint 10 — Basic Synth ✅
- [x] BasicSynth (Dual Osc + ADSR + Filter)
- [x] SimpleFilter (LP/HP/BP with resonance)

### Future
- [ ] Sample layering
- [ ] Preset system

## 🔌 Phase 4 : VST Support

### Sprint 11 — Core Infrastructure ✅
- [x] PluginManager singleton (scan, cache, async creation)
- [x] VST3EffectSlot wrapper (AudioPluginInstance → EffectSlot)
- [x] Platform-specific search paths
- [x] Deadman's pedal crash detection

### Sprint 12 — UI Integration ✅
- [x] PluginEditorWindow (native plugin UI hosting)
- [x] GenericPluginEditor (fallback parameter grid)
- [x] EffectRackPanel VST3 combo integration
- [x] MainComponent PluginManager initialization

---

## 🎨 Phase 5 : UI/UX Pro-Level ✅

### Sprint 13 — Theme & Visual Overhaul ✅
- [x] Theme system (Shadows, Gradients, TrackColours, Glow namespaces)
- [x] DrawingHelpers utilities (shadow, glow, gradient rendering)
- [x] ModernLookAndFeel (beveled buttons, enhanced knobs)
- [x] LED-style VU meters (24 segments, FL Studio style)
- [x] Gradient waveforms (per-track colors)
- [x] ClipComponent polish (shadows, gradient bg, header dark)
- [x] TransportBar (inner shadow, time display glow)
- [x] EffectSlotComponent (shadow, header gradient, bypass glow)
- [x] ModalOverlay (animated backdrop, vignette)
- [x] FirstRunDialog (gradient, accent bar glow)
- [x] CI/CD GitHub Actions (Linux + Windows builds)

---

## 🏗️ Phase 6 : Infrastructure & Quality ✅

### Sprint 14 — Bug Fixes ✅
- [x] Fix audio playback (buffer loading into AudioClip)
- [x] Fix track renaming (double-click inline edit)
- [x] Fix timeline ruler alignment (headerWidth offset)

### Sprint 14 — Audit v14 Remediation ✅
- [x] Unit test framework (JUCE UnitTest)
- [x] tests/ directory + Source/Tests/
- [x] CMakeLists.txt BUILD_TESTS option
- [x] .pre-commit-config.yaml
- [x] LICENSE (GPL-3.0)
- [x] CONTRIBUTING.md
- [x] SECURITY.md
- [x] docs/architecture.md
- [x] docs/quickstart.md
- [x] **Audit: 6/18 → 16/18 (Grade S Gold)**

---

## 🛠️ Phase 7 : UX Fixes ✅

### Sprint 15 — UX Audit & Fixes ✅
- [x] Fix AudioEngine initialization race condition
- [x] Auto-select first track on startup
- [x] Wire effect callbacks (onEffectAdded/Removed/Bypass/Parameter)
- [x] Track selection in Timeline (mouseDown + highlight)
- [x] Timeline↔Mixer selection sync
- [x] Clip tooltips (name + duration)
- [x] Clip hover effect (subtle glow)

### Sprint 16 — Asset Browser ✅
- [x] AssetBrowser component (left panel)
- [x] File tree with audio file filtering
- [x] Recent files list (last 10)
- [x] Folder bookmarks (Music, Downloads, Desktop, Home)
- [x] Double-click to import
- [x] Drag support from browser

---

## 💾 Phase 8 : Persistence ✅

### Sprint 17 — Project Save/Load ✅
- [x] ProjectSerializer (JSON serialization)
- [x] .dawc file format
- [x] Full project state (tempo, sample rate, master volume)
- [x] Track serialization (volume, pan, mute, solo, colour)
- [x] Clip data (source file, gain, position, duration)
- [x] Effect chain persistence (all parameters)
- [x] VST3 state (base64 encoded binary)
- [x] File menu integration (Save/Save As/Open)
- [x] Unsaved changes prompt

### Sprint 18 — Preset System ✅
- [x] Preset data structure (JSON format)
- [x] PresetManager (scan, save, load, apply)
- [x] PresetBrowser UI (CallOutBox popup)
- [x] Preset button "P" in EffectSlotComponent
- [x] Factory presets (15+ for EQ, Comp, Reverb, Delay, Kick, Synth)
- [x] Category filtering

---

## 🎨 Phase 9 : UX Overhaul ✅

### Sprint 19 — Drag-Drop & Context Menus ✅
- [x] Fix transport icons (Path drawing instead of UTF-8)
- [x] Fix button states after playback ends
- [x] Fix UTF-8 encoding issues (em-dash in EffectRack/Mixer)
- [x] Wire track rename callback (Timeline → Mixer → AudioEngine)
- [x] Drop to specific track from coordinates
- [x] Create 8 default tracks on new project
- [x] Visual drop zone feedback (track highlight on drag)
- [x] Clip inter-track dragging (vertical drag)
- [x] Keyboard shortcuts (Space, M, S, Ctrl+S/O/N/T)
- [x] Context menus (clips: Delete/Duplicate, tracks: Rename/Color/Delete)
- [x] Track colour picker (12-colour palette)

---

## ✂️ Phase 10 : Advanced Editing & BPM Workflow ✅

### Sprint E — Clip Editing ✅
- [x] Split clip at playhead (Ctrl+B)
- [x] Split clip at click position (Alt+Click)
- [x] Trim handles on clip edges
- [x] Drag left edge adjusts startTime + sourceOffset
- [x] Drag right edge adjusts duration
- [x] Trim cursor (↔) on edge hover
- [x] Paste at click position (5s timeout)

### Sprint E — BPM & Beat Grid ✅
- [x] BPM sync Project ↔ TransportController ↔ Timeline
- [x] Beat grid on ruler (bar lines + beat lines)
- [x] Bar numbers display
- [x] Beat-based snap (1/Bar, 1/Beat, 1/2, 1/4, 1/8, 1/16)
- [x] Snap selector dropdown in TransportBar
- [x] Position display toggle (Time ↔ Bar:Beat:Tick)
- [x] 960 PPQ resolution for ticks

### Sprint E — Delay Tempo Sync ✅
- [x] Sync button in Delay effect UI
- [x] Note value selector (1/2, 1/4, 1/8, 1/16, dotted)
- [x] Tempo propagation to EffectRackPanel/EffectSlotComponent

---

## 🎯 Phase 11 : Drag & Drop Workflow ✅

### Sprint F — Clip Drag Freedom ✅
- [x] Drag clips to any track (absolute index, not delta)
- [x] onQueryTrackAtY callback for track detection
- [x] currentTrackIndex tracking in ClipComponent

### Sprint F — Browser to Timeline ✅
- [x] DragAndDropTarget interface in TimelinePanel
- [x] isInterestedInDragSource() for "AudioFile" description
- [x] itemDragMove() with visual feedback
- [x] itemDropped() with file import

### Sprint F — Smart Import ✅
- [x] Double-click imports to selected track (not Track 1)
- [x] Import at playhead position
- [x] importFileToTrack() centralized helper

### Sprint F — Visual Feedback ✅
- [x] Drop target highlighting during drag
- [x] Clear drop targets on drag end

---

## ⏪ Phase 12 : Undo/Redo & Advanced Editing ✅

### Sprint G — Undo/Redo System ✅
- [x] UndoableCommand base class (merge support 500ms)
- [x] UndoManager (dual-stack, max 100 actions)
- [x] MoveClipCommand (multi-selection)
- [x] DeleteClipsCommand (full restore on undo)
- [x] AddClipCommand (duplicate, paste, import)
- [x] TrimClipCommand (merge support)
- [x] MoveClipToTrackCommand

### Sprint G — Keyboard Shortcuts ✅
- [x] Ctrl+Z — Undo
- [x] Ctrl+Y / Ctrl+Shift+Z — Redo
- [x] Ctrl+D — Duplicate selected clips (multi-selection)
- [x] Ctrl+Shift+A — Select all clips globally

### Sprint G — Snap Improvements ✅
- [x] Snap to clip edge (start/end of other clips)
- [x] 100ms threshold for clip edge snap
- [x] Grid snap + clip edge snap combined

---

## 🎬 Phase 13 : Pro Editing Workflow ✅

### Sprint H — Fades & Crossfades ✅
- [x] Fade In/Out duration fields in Clip model
- [x] FadeType enum (Linear, Exponential, SCurve, Logarithmic)
- [x] DSP fade processing in AudioClip::getNextAudioBlock()
- [x] Fade curve functions (applyFadeCurve)
- [x] Fade handles UI on ClipComponent (draggable triangles)
- [x] Auto-crossfade detection (overlap ≤ 2s)
- [x] FadeClipCommand for undo/redo support
- [x] Fade data serialized in .dawc project files

### Sprint H — Markers System ✅
- [x] Marker model (id, name, time, colour, shortcut 1-9)
- [x] Visual markers on TimelineRuler (flag + vertical line)
- [x] Shift+M — Add marker at playhead
- [x] Numpad 1-9 — Jump to marker by shortcut
- [x] Click marker to jump to position
- [x] Right-click marker to delete
- [x] Markers serialized in .dawc project files

### Sprint H — Zoom Enhancements ✅
- [x] Ctrl+0 — Zoom to fit all clips
- [x] Ctrl+1 — Zoom to selection (10% padding)
- [x] zoomToTimeRange(start, end) helper method

### Sprint H — Clip Grouping ✅
- [x] ClipGroup model (id, name, clips, colour)
- [x] Ctrl+G — Group selected clips
- [x] Ctrl+Shift+G — Ungroup selected clips
- [x] Groups persisted in .dawc project files
- [x] Group management in Project (createGroup, dissolveGroup, getGroupForClip)

### Sprint H — Ripple Edit Mode ✅
- [x] EditMode enum (Normal, Ripple)
- [x] R key — Toggle ripple edit mode
- [x] Delete in ripple mode shifts subsequent clips
- [x] Full undo support for ripple operations
- [x] AudioClip::setStartTime() for ripple shifts
- [x] AudioTrack::updateClipStartTime() for ripple shifts

---

## 💡 Ideas (Backlog Long Terme)

- [ ] Piano roll / MIDI editor
- [ ] Automation lanes
- [ ] Sidechain compression
- [ ] Spectrum analyzer
- [ ] Mastering chain presets
- [x] ~~Project save/load (JSON)~~ ✅ Sprint 17
- [x] ~~Undo/redo system~~ ✅ Sprint G
