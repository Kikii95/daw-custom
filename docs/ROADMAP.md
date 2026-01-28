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
- [ ] **Phase 5 : Project Save/Load** (Sprint 13-14)
  - Serialization JSON du projet complet
  - Restauration tracks + effets + clips + VST3
  - Autosave optionnel

### Avril-Mai
- [ ] **Phase 6 : Preset System** (Sprint 15-16)
  - Sauvegarder/charger presets effets + synths
  - Preset browser UI
  - Factory presets

---

## Q3 2026 — Polish

### Juillet-Août
- [ ] UI polish & themes
- [ ] Project save/load
- [ ] Undo/redo

### Septembre
- [ ] Beta release v1.0
- [ ] Documentation utilisateur

---

## Future

- Piano roll / MIDI editor
- MIDI support complet
- Automation lanes
- Mastering tools
- Spectrum analyzer
