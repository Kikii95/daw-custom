# DAW Custom — Instructions Projet

## 🎯 Projet

**Type** : DAW (Digital Audio Workstation)
**Stack** : C++ + JUCE Framework (GPL)
**Objectif** : Mashups + création de kicks/sons custom

---

## 🏗️ Architecture

```
daw-custom/
├── .claude/              # Instructions Claude
├── docs/                 # Documentation
│   ├── adr/             # Architecture Decision Records
│   ├── BACKLOG.md
│   ├── ROADMAP.md
│   └── CHANGELOG.md
├── Source/              # Code source JUCE
│   ├── Main.cpp
│   ├── MainComponent.cpp/h
│   ├── Audio/           # Audio engine, effects
│   └── UI/              # Interface components
├── Resources/           # Assets (icons, presets)
├── _reference/          # Code legacy (XAudio2) pour consultation
└── CMakeLists.txt
```

---

## 📚 Docs à Maintenir

### Docs Git (source) → sync Obsidian

| Doc | Chemin Git | Quand mettre à jour |
|-----|------------|---------------------|
| Backlog | `docs/BACKLOG.md` | Nouvelles tâches |
| Roadmap | `docs/ROADMAP.md` | Changement planning |
| Changelog | `docs/CHANGELOG.md` | Chaque release |

### Docs Obsidian-only

| Doc | Chemin Obsidian |
|-----|-----------------|
| Note projet | `Projects/Perso/daw-custom/` |
| Logs | `Projects/Perso/daw-custom/_Logs/` |
| Scope MVP | `Projects/Perso/daw-custom/Documents/Scope-MVP.md` |

---

## 🔧 Conventions

### Code Style
- C++20
- Naming: PascalCase classes, camelCase methods/vars
- JUCE coding standards

### Commits
- Conventional commits: `feat:`, `fix:`, `docs:`, `refactor:`
- Pas de traces IA

### Build
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

---

## 🎹 Features MVP — État Actuel

| Phase | Feature | Status |
|-------|---------|--------|
| **Phase 1** | Mixer (timeline, volume/pan, import/export) | ✅ v0.1.0 |
| **Phase 2** | Effects (EQ, Compressor, Reverb, Delay) | ✅ Sprint 6-7 |
| **Phase 3** | Sound Design (Kick, Synth, Filter) | ✅ Sprint 8-10 |
| **Phase 4** | VST3 Support (scan, load, UI) | ✅ Sprint 11-12 |

**MVP TERMINÉ** — Prochaines phases : Preset system, Project save/load

---

## 📦 Dépendances

- JUCE 7+ (via CMake FetchContent)
- C++20 compiler (GCC 11+, Clang 14+, MSVC 2022)

---

## ⚠️ Notes

- Code legacy dans `_reference/` = XAudio2 Windows-only, pour référence DSP uniquement
- Licence GPL = projet open source
