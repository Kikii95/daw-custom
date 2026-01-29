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

## 💡 Ideas (Backlog Long Terme)

- [ ] Piano roll / MIDI editor
- [ ] Automation lanes
- [ ] Sidechain compression
- [ ] Spectrum analyzer
- [ ] Mastering chain presets
- [ ] Project save/load (JSON)
- [ ] Undo/redo system
