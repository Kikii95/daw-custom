# Contributing to DAW Custom

Thank you for your interest in contributing to DAW Custom!

## Code Style

- **C++20** standard
- **Naming**: PascalCase for classes, camelCase for methods/variables
- **Framework**: Follow JUCE coding conventions
- **Formatting**: Use clang-format with project config

## Development Setup

```bash
# Clone
git clone https://github.com/Kikii95/daw-custom.git
cd daw-custom

# Build
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j$(nproc)

# Run
./build/DAWCustom_artefacts/Debug/DAW\ Custom
```

## Commit Messages

Use [Conventional Commits](https://www.conventionalcommits.org/):

- `feat:` — New feature
- `fix:` — Bug fix
- `docs:` — Documentation changes
- `refactor:` — Code refactoring
- `test:` — Adding tests
- `chore:` — Maintenance tasks

Examples:
```
feat(effects): add parametric EQ with 3 bands
fix(transport): resolve playback position drift
docs: update README with build instructions
```

## Pull Request Process

1. Fork the repository
2. Create a feature branch (`git checkout -b feat/my-feature`)
3. Make your changes
4. Ensure it compiles: `cmake --build build`
5. Commit with clear message
6. Push and open a Pull Request
7. Describe your changes in the PR description

## Architecture Overview

See [docs/architecture.md](docs/architecture.md) for project structure.

## Questions?

Open an issue for questions or discussions.
