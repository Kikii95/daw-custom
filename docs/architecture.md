# Architecture Overview

DAW Custom follows a layered architecture separating concerns between audio processing, data model, and user interface.

## Layer Diagram

```
┌─────────────────────────────────────────────────────┐
│                    UI Layer                          │
│  ┌─────────────┐ ┌─────────────┐ ┌───────────────┐  │
│  │ TimelinePanel│ │ MixerPanel  │ │EffectRackPanel│ │
│  │  - TrackLane │ │ -ChannelStrip│ │-EffectSlotComp│ │
│  │  - ClipComp  │ │ -MeterComp  │ │               │  │
│  └─────────────┘ └─────────────┘ └───────────────┘  │
├─────────────────────────────────────────────────────┤
│                   Model Layer                        │
│  ┌─────────────┐ ┌─────────────┐ ┌───────────────┐  │
│  │   Project   │ │    Track    │ │     Clip      │  │
│  │  - tracks[] │ │  - clips[]  │ │ - startTime   │  │
│  │  - tempo    │ │  - volume   │ │ - duration    │  │
│  └─────────────┘ └─────────────┘ └───────────────┘  │
├─────────────────────────────────────────────────────┤
│                   Audio Layer                        │
│  ┌─────────────┐ ┌─────────────┐ ┌───────────────┐  │
│  │ AudioEngine │ │ AudioMixer  │ │  AudioTrack   │  │
│  │  - device   │ │  - tracks[] │ │  - clips[]    │  │
│  │  - source   │ │  - master   │ │  - effects    │  │
│  └─────────────┘ └─────────────┘ └───────────────┘  │
│                                                      │
│  ┌─────────────┐ ┌─────────────┐ ┌───────────────┐  │
│  │  AudioClip  │ │ EffectChain │ │  EffectSlot   │  │
│  │  - buffer   │ │  - slots[]  │ │  (GainEffect, │  │
│  │  - position │ │             │ │   EQ, Comp...)│  │
│  └─────────────┘ └─────────────┘ └───────────────┘  │
└─────────────────────────────────────────────────────┘
```

## Directory Structure

```
Source/
├── Main.cpp              # Application entry point
├── MainComponent.cpp/h   # Root component, menu bar
│
├── Audio/                # Audio processing
│   ├── AudioEngine.cpp/h      # Device management
│   ├── AudioMixer.cpp/h       # Multi-track mixing
│   ├── AudioTrack.cpp/h       # Single track playback
│   ├── AudioClip.cpp/h        # Audio buffer source
│   ├── TransportController.h  # Play/pause/seek
│   └── DSP/                   # Effects
│       ├── EffectSlot.h       # Base effect class
│       ├── EffectChain.cpp/h  # Effect container
│       ├── GainEffect.cpp/h
│       ├── ParametricEQ.cpp/h
│       ├── CompressorEffect.cpp/h
│       ├── ReverbEffect.cpp/h
│       └── DelayEffect.cpp/h
│
├── Model/                # Data structures
│   ├── Project.cpp/h     # Project container
│   ├── Track.h           # Track metadata
│   └── Clip.h            # Clip metadata
│
├── UI/                   # User interface
│   ├── Theme/            # Visual styling
│   │   ├── AppTheme.h
│   │   └── ModernLookAndFeel.cpp/h
│   ├── Timeline/         # Track visualization
│   │   ├── TimelinePanel.cpp/h
│   │   ├── TimelineRuler.cpp/h
│   │   ├── TrackLane.cpp/h
│   │   └── ClipComponent.cpp/h
│   ├── Mixer/            # Audio mixing
│   │   ├── MixerPanel.cpp/h
│   │   ├── ChannelStrip.cpp/h
│   │   └── MeterComponent.cpp/h
│   ├── Transport/        # Playback controls
│   │   └── TransportBar.cpp/h
│   └── Effects/          # Effect UI
│       ├── EffectRackPanel.cpp/h
│       └── EffectSlotComponent.cpp/h
│
├── File/                 # File I/O
│   ├── AudioFileLoader.cpp/h
│   └── AudioFileExporter.cpp/h
│
└── Plugins/              # VST3 support
    ├── PluginManager.cpp/h
    └── VST3EffectSlot.cpp/h
```

## Data Flow

### Audio Playback

```
TransportController.play()
       │
       ▼
AudioMixer.getNextAudioBlock()
       │
       ├──► AudioTrack[0].getNextAudioBlock()
       │         │
       │         ├──► AudioClip[0].getNextAudioBlock()
       │         ├──► AudioClip[1].getNextAudioBlock()
       │         │
       │         └──► EffectChain.processBlock()
       │
       ├──► AudioTrack[1].getNextAudioBlock()
       │
       ▼
AudioEngine → Audio Device
```

### UI Updates

```
User Action (click, drag)
       │
       ▼
Component callback (onClipDragged, onVolumeChanged)
       │
       ├──► Update Model (Track, Clip)
       │
       └──► Update Audio (AudioTrack, AudioClip)
              │
              ▼
         Repaint UI
```

## Key Design Decisions

See `docs/adr/` for Architecture Decision Records:

- **ADR-001**: JUCE framework selection (cross-platform, GPL)
- Effect chain architecture (insert-based, per-track)
- VST3 support via JUCE AudioPluginHost
