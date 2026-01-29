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

## 💡 Ideas (Backlog Long Terme)

- [ ] Piano roll / MIDI editor
- [ ] Automation lanes
- [ ] Sidechain compression
- [ ] Spectrum analyzer
- [ ] Mastering chain presets
- [x] ~~Project save/load (JSON)~~ ✅ Sprint 17
- [ ] Undo/redo system
