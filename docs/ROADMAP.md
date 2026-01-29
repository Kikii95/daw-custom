# Roadmap — DAW Custom

## 🎯 Vision

DAW personnel pour mashups et sound design, avec support VST gratuits.

---

## Q1 2026 — Foundation ✅

### Janvier (FAIT)
- [x] Choix stack (JUCE GPL)
- [x] ADR-001 documenté
- [x] Setup projet JUCE + CMake
- [x] Audio I/O (AudioEngine + AudioDeviceManager)
- [x] Première fenêtre GUI (MainLayout)
- [x] Import audio multi-formats (WAV, MP3, FLAC)
- [x] Timeline avec waveforms
- [x] Transport controls (play/pause/stop)
- [x] Mixer multi-pistes (volume, pan, mute/solo)
- [x] VU-mètres (MeterComponent)
- [x] Export audio (WAV, FLAC)
- [x] **Phase 1 MVP TERMINÉE**

### Février-Mars
- [x] Phase 2 Sprint 6 : Effects Core (EQ, Compressor, Reverb, Delay) ✅
- [x] Effect chain architecture (EffectSlot, EffectChain)
- [x] Phase 2 Sprint 7 : Effect rack UI (EffectRackPanel + EffectSlotComponent) ✅

---

## Q2 2026 — Sound Design

### Janvier ✅
- [x] Phase 3 Sprint 8 : Core Synthesis ✅
  - Oscillators.h, OscillatorEffect, EnvelopeEffect
- [x] Phase 3 Sprint 9 : Kick Designer ✅
  - KickDesigner (808-style kick synth)
- [x] Phase 3 Sprint 10 : Basic Synth ✅
  - BasicSynth (Dual Osc + ADSR + Filter)
  - SimpleFilter (LP/HP/BP)

### Février-Mars
- [x] Phase 4 Sprint 11 : Plugin Infrastructure ✅
  - PluginManager (scan, cache, async creation)
  - VST3EffectSlot wrapper
- [x] Phase 4 Sprint 12 : Plugin UI Integration ✅
  - PluginEditorWindow, GenericPluginEditor
  - EffectRackPanel integration
- [x] **Phase 4 TERMINÉE** — VST3 Support opérationnel

### Février-Mars (suite)
- [x] ~~**Phase 5 : Project Save/Load**~~ → Moved to Sprint 17 ✅
- [x] ~~**Phase 6 : Preset System**~~ → Moved to Sprint 18 ✅

---

## Q1 2026 — UI/UX Overhaul ✅

### Janvier (FAIT)
- [x] **Sprint 13 : Pro-Level UI/UX** ✅
  - Theme system (Shadows, Gradients, TrackColours, Glow)
  - DrawingHelpers utilities
  - LED-style VU meters (FL Studio)
  - Gradient waveforms with per-track colors
  - Beveled 3D buttons, enhanced knobs
  - Modal overlay with animations
  - CI/CD GitHub Actions (Linux + Windows)

- [x] **Sprint 14 : Bug Fixes + Infrastructure** ✅
  - Fix audio playback (buffer → AudioClip → AudioTrack)
  - Fix track renaming (double-click inline edit)
  - Fix timeline ruler alignment
  - Unit test framework (JUCE UnitTest)
  - Audit v14 remediation: 6/18 → 16/18 (Grade S Gold)
  - LICENSE, CONTRIBUTING, SECURITY, docs

---

## Q1 2026 — UX Fixes ✅

### Janvier
- [x] **Sprint 15 : UX Audit & Fixes** ✅
  - Fix AudioEngine race condition
  - Track selection in Timeline + sync
  - Clip tooltips + hover effects
  - Auto-select first track
- [x] **Sprint 16 : Asset Browser** ✅
  - File tree with audio filtering
  - Recent files + bookmarks
  - Double-click import

---

## Q1 2026 — Persistence ✅

### Janvier
- [x] **Sprint 17 : Project Save/Load** ✅
  - ProjectSerializer (JSON serialization)
  - .dawc file format with versioning
  - Full state persistence (tracks, clips, effects, VST3)
  - File menu integration (Save/Save As/Open)
- [x] **Sprint 18 : Preset System** ✅
  - PresetManager + PresetBrowser UI
  - Factory presets (15+ for EQ, Comp, Reverb, Delay, Kick, Synth)
  - Effect preset button "P"

---

## Q1 2026 — UX Overhaul ✅

### Janvier
- [x] **Sprint 19 : Drag-Drop & Context Menus** ✅
  - Transport icons (Path drawing)
  - Drop to specific track
  - Visual drop feedback
  - Clip inter-track dragging
  - 8 default tracks
  - Context menus (clips/tracks)
  - Keyboard shortcuts (Space, M, S, Ctrl+S/O/N/T)
  - Track colour picker

---

## Q2-Q3 2026 — Polish

### Février-Mars
- [x] ~~Project save/load (JSON serialization)~~ ✅ Sprint 17
- [x] ~~Preset system (effects + synths)~~ ✅ Sprint 18
- [ ] Undo/redo
- [ ] MIDI support

### Avril-Juin
- [ ] Beta release v1.0
- [ ] Documentation utilisateur

---

## Future

- Piano roll / MIDI editor
- MIDI support complet
- Automation lanes
- Mastering tools
- Spectrum analyzer
