# CLAUDE.md: Project rules for Launcher

## Language

User communicates in Italian: respond in Italian. Commit messages in English, 1 line only. All commits must have Signed-off-by (`git commit -s`).

## Code style

No emojis or symbols anywhere, ASCII only. No separator comment lines in any file. No `|| true` or error-swallowing hacks in CI. Default to writing no comments unless the WHY is non-obvious. Use KDoc-style comments before classes and functions only.

## Documentation style

Use `: ` instead of ` — ` in documentation and comments. Example: `CoreError: represents errors from the Rust core library`. No bullet points or numbered lists in documentation, use plain text with headings.

## Project layout (C++ standard)

`src/` for main C++ application source. `libraries/` for helper libraries and submodules. `core/` for the Rust core library (name is `core`, NOT `launcher-core`). `res/` for resources like icons, desktop files, and config. `cmake/` for CMake modules. `buildconfig/` for build configuration. `tests/` for test files. `flatpak/` and `snap/` for packaging manifests.

## Data vs code separation

All configuration data lives in `res/launcher-config.yaml`. Loaded at build time via `cmake/ParseLauncherConfig.cmake`. Never hardcode URLs or config values in source files. Runtime data goes in `.yaml` or `.ini` files.

## Rust core (`core/`)

Variable names must be semantic: `compressed_data`, `output_buffer`, `file_size_human`. Never use generic names like `data`, `result`, `buf`, `s`, `tmp`. CI job named exactly `core` (not `launcher-core`). Triggers only on `core/**` path changes.

## CI/CD

Build runs in distrobox container `lb` (Fedora Toolbox). MSVC removed from CI, only MinGW for Windows. Release must not be draft. All CI jobs must pass, fix failures at root cause. CI matrix: Linux, Linux-aarch64, Windows-MinGW-w64, Windows-MinGW-arm64, macOS.

## Release

Tag format: `v111.0.x`. Release uploads binaries, `.nix`, and all dotfiles. Version numbers in `res/launcher-config.yaml`.

## Commits

Format: `type: short description` (1 line) + blank line + `Signed-off-by: ...`. No emojis, no multi-line messages. For non-owner commits, preserve original author's name and email.
