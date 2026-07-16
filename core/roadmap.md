# Core Rust Roadmap

## Vision

The goal is to migrate all non-UI logic from C++ to Rust. The C++ layer will be reduced to Qt UI code and thin FFI wrappers. Rust becomes the single source of truth for business logic, data processing, and system interactions.

## Current State

The Rust core already handles compression, JSON processing, filesystem operations, string utilities, and markdown rendering. These modules are exposed to C++ via FFI bindings in `ffi.rs`.

## Phase 1: Foundation

### Hashing and Checksums

Hashing is the most pervasive dependency in the codebase. Every download, every file transfer, every integrity check relies on cryptographic hashes. Moving this to Rust eliminates an entire class of buffer overflow vulnerabilities in C++ hash implementations.

The module provides SHA-256, SHA-512, and MD5 computation over byte slices, file streams, and async readers. It integrates with the existing FFI layer so C++ callers can verify downloads without managing memory manually.

### Archive Processing

Zip and tar extraction currently lives in C++ libarchive wrappers. Rust's `zip` and `tar` crates are memory safe by construction and handle edge cases that libarchive silently corrupts.

The archive module reads zip, tar.gz, and tar.xz archives. It exposes streaming extraction that writes directly to disk without buffering entire archives in memory. This is critical for modpack installation where archives can exceed 500 MB.

## Phase 2: Network Layer

### HTTP Client

The current Qt-based download manager mixes networking logic with UI progress reporting. A Rust HTTP client decouples these concerns.

The client handles retries, resume from partial downloads, parallel chunked transfers, and certificate pinning. It exposes a callback interface for progress reporting that the C++ UI layer can hook into without owning the connection lifecycle.

### API Clients

Each mod platform (CurseForge, Modrinth, FTB, Technic, ATLauncher) has its own HTTP API. Currently these are scattered across C++ classes with duplicated error handling and serialization logic.

Rust API clients use `serde` for automatic deserialization. Each platform gets a dedicated module that handles authentication, rate limiting, and response parsing. The C++ layer calls a single function to fetch a mod list or install a pack.

## Phase 3: Instance Management

### Instance Configuration

Instance metadata lives in INI files that C++ reads with custom parsers. Rust can parse these with proper error recovery and validation.

The configuration module reads and writes `instance.cfg` and `pack.json` files. It validates field types, handles migration from older formats, and provides typed access to instance properties. This eliminates the stringly-typed settings API in C++.

### Component Resolution

Minecraft version resolution involves fetching Mojang manifests, resolving dependency trees, and merging component overrides. This is currently spread across `PackProfile`, `Component`, and `VersionFile` classes.

Rust component resolution fetches manifests, builds a dependency graph, and produces a flat list of libraries and assets to download. The C++ layer receives a resolved manifest and passes it to the download manager.

## Phase 4: Mod Ecosystem

### Mod Metadata

CurseForge and Modrinth expose mod metadata through REST APIs. The current C++ implementation parses JSON responses into loosely typed structures.

Rust mod metadata uses strongly typed structs with `serde`. Each platform's response schema is defined once and validated at compile time. Invalid responses produce structured errors instead of silent fallbacks.

### Mod Installation

Installing a mod involves downloading archives, extracting contents, merging with existing files, and updating instance metadata. This workflow is currently duplicated across Flame, Modrinth, and Technic installers.

Rust mod installation defines a common `ModInstaller` trait. Each platform provides a concrete implementation that handles its specific archive format and metadata structure. The C++ layer orchestrates the high-level flow while Rust handles file operations.

## Phase 5: Authentication

### Account Management

Authentication tokens, session cookies, and encryption keys currently live in C++ classes that mix storage with cryptographic operations.

Rust account management stores credentials in OS keychains where available, falls back to encrypted file storage, and handles token refresh cycles. The C++ layer presents login dialogs and calls into Rust for token acquisition.

### Platform Authentication

Microsoft OAuth, Ely.by, and Mojang authentication each have distinct flows. These are currently implemented as separate C++ classes with shared utility functions.

Rust authentication modules implement each flow as a state machine. Token exchange, device code flow, and session validation are handled internally. The C++ layer triggers authentication and receives a session token.

## Phase 6: Settings and Persistence

### Settings Management

The current settings system uses `QSettings` and custom INI parsers. This creates a dependency on Qt for basic key-value storage.

Rust settings management reads and writes INI and YAML files. It provides typed getters with default values, change notification, and atomic writes. The C++ layer wraps this in a `QObject` for Qt property binding.

### Data Migration

When the launcher upgrades, instance formats and settings schemas may change. Migration logic currently lives in `DataMigrationTask`.

Rust data migration defines versioned schema transformations. Each migration is a pure function that transforms old data to new format. The C++ layer calls the migration at startup and handles any user prompts.

## Integration Strategy

Each phase produces a Rust library that links statically into the launcher binary. The C++ layer includes a generated header from `cbindgen` and calls Rust functions through `extern "C"` wrappers.

The FFI boundary is kept minimal. Rust owns all data structures and returns them as opaque pointers. C++ dereferences pointers only when passing data to Qt widgets. This prevents use-after-free bugs at the language boundary.

## Testing Strategy

Each Rust module includes unit tests that run in `cargo test`. Integration tests verify FFI correctness by calling Rust functions from C++ test harnesses. The CI pipeline runs Rust tests independently of the C++ build, catching regressions early.

## Completion Criteria

The migration is complete when:
- All business logic lives in Rust
- C++ contains only Qt UI code and FFI wrappers
- No C++ code directly manipulates raw memory from Rust allocations
- Every Rust module has >90% test coverage
- The binary size does not increase by more than 5% from static linking
