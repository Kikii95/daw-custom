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

## 🎛️ Phase 2 : Effects (EN COURS)

### Sprint 6 — Architecture + Effets Core ✅
- [x] Architecture effect chain (EffectSlot, EffectChain)
- [x] GainEffect (volume + constant power pan)
- [x] EQ paramétrique (3 bandes: Low/Mid/High)
- [x] Compressor (threshold, ratio, attack, release, makeup)
- [x] Reverb (Freeverb wrapper)
- [x] Delay (port legacy XAudio2 avec smoothing)
- [x] Effect bypass support
- [x] Intégration AudioTrack

### Sprint 7 — UI (À FAIRE)
- [ ] Effect rack UI panel
- [ ] Effect slot component (knobs, bypass)

## 🎹 Phase 3 : Sound Design

- [ ] Kick designer (sine + noise + envelope)
- [ ] Basic synth (oscillateurs, filtres, ADSR)
- [ ] Sample layering
- [ ] One-shot export
- [ ] Preset system

## 🔌 Phase 4 : VST Support

- [ ] VST3 plugin scanner
- [ ] VST plugin hosting
- [ ] Plugin parameter automation
- [ ] Preset management

---

## 💡 Ideas (Backlog Long Terme)

- [ ] Piano roll / MIDI editor
- [ ] Automation lanes
- [ ] Sidechain compression
- [ ] Spectrum analyzer
- [ ] Mastering chain presets
- [ ] Project save/load (JSON)
- [ ] Undo/redo system
