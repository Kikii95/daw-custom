# ADR-001: Choix Stack — JUCE Framework

**Date**: 2026-01-28
**Status**: Accepted

## Contexte

Création d'un DAW custom pour mashups et sound design.
Critères : performance, VST support, UI belle, licence gratuite.

## Décision

**C++ + JUCE Framework (licence GPL)**

## Alternatives Rejetées

- **Rust + Tauri** : UI plus belle mais pas de VST natif
- **C++ + miniaudio** : Pas de VST natif
- **Electron** : Latence audio trop élevée

## Raisons

1. VST3 hosting natif
2. Licence GPL = gratuit pour projet open source
3. Framework mature (utilisé par Bitwig, Vital, Surge)
4. Cross-platform

## Conséquences

- Code doit être open source (GPL)
- Learning curve JUCE
- Architecture différente du code legacy
