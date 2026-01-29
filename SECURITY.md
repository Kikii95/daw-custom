# Security Policy

## Scope

DAW Custom is a personal audio workstation project. Security concerns primarily relate to:

- Audio file parsing (WAV, MP3, FLAC, OGG)
- VST3 plugin loading and sandboxing
- Project file serialization

## Reporting a Vulnerability

If you discover a security vulnerability:

1. **Do not** open a public issue
2. Email the maintainer directly with details
3. Include steps to reproduce if possible
4. Allow reasonable time for a fix before disclosure

## Supported Versions

| Version | Supported |
|---------|-----------|
| 0.1.x   | Yes       |

## Security Best Practices

When using DAW Custom:

- Only load VST3 plugins from trusted sources
- Be cautious with audio files from unknown origins
- Keep the application updated
