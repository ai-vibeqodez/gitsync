# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- implement `--update` / `-u` feature to check for and apply updates from GitHub releases
- utils module with version parsing and comparison functions
- comprehensive test suite for utils module (7 tests covering version parsing, comparison, and JSON parsing)
- Windows binary metadata: version info, icon embedding, and code signing support

## [0.4.0] - 2026-11-22

### Added

- add multi-platform binary builds to release workflow
- rewrite gitsync from Rust to C++23

### Changed

- use reusable-release.yml like tpl
- Merge branch 'main' of github.com:ai-warevo/gitsync
- slop
- update CHANGELOG for v0.3.0 release
- version 0.3.0
- version 0.2.0
- fix
- use aw_add_library instead of aw_core_library
- replace vendored dependencies with FetchContent
- update to new workflow structure
- add .clang-tidy and .editorconfig config files
- remove unnecessary documentation files
- add final project completion summary
- use C++23 instead of Rust

### Fixed

- Windows CMake presets and build-release target
- override changelog-release defaults (remove boilerplate)
- Windows compatibility - use _popen/_pclose and safe getenv
- use ilammy/msvc-dev-cmd and cmake-build action for Windows

## [0.3.0] - 2026-11-23

### Added
- Multi-platform release binaries (Linux + Windows) in release workflow
- Windows MSVC build support with ilammy/msvc-dev-cmd action
- Safe environment variable access (_dupenv_s on Windows, getenv on Unix)

### Changed
- Release workflow now builds optimized binaries without tests
- Improved CMake configuration for Release builds (-DGITSYNC_BUILD_TESTS=OFF)

### Fixed
- Windows compatibility: replaced popen/pclose with _popen/_pclose
- Windows compatibility: replaced unsafe getenv with _dupenv_s
- Windows compatibility: proper HOME detection (USERPROFILE on Windows)

## [0.2.0] - 2026-11-22

### Added
- Complete rewrite from Rust to C++23 with modern language features
- CMake build system with presets (clang18-libcxx, release, msvc)
- Makefile CI contract (ci-fast, ci-deep, ci-coverage, build-release)
- Comprehensive test suite using Catch2 v3 (11 unit tests, 100% coverage)
- Static analysis via clang-tidy and code formatting via clang-format
- Multi-platform CI/CD with GitHub Actions
- Coverage badge generation and reporting

### Changed
- Build system: Cargo → CMake 3.23+ with presets
- Dependencies: managed via CMake FetchContent (zero vendored code)
- Logging: env_logger → spdlog with structured logging
- Async runtime: Tokio → synchronous C++ (future async support planned)

### Maintained
- Full configuration compatibility with Rust version (v0.1.x)
- JSON configuration format with PascalCase field names
- Three-layer config merge hierarchy (global → org → repo)
- SSH known_hosts management
- Multi-remote synchronization logic
- CLI argument compatibility (-v, -h, -u, -s)

## [0.1.0] - 2024-01-01 (Rust)

### Added
- Initial Rust implementation
- Multi-layer configuration system
- SSH known_hosts warm-up
- Git repository synchronization across multiple remotes
- Template-based branch naming
- CLI with version, help, update, and skip-host options

[Unreleased]: https://github.com/ai-warevo/gitsync/compare/v0.4.0...HEAD
[0.4.0]: https://github.com/ai-warevo/gitsync/compare/v1.1.10...v0.4.0
[0.3.0]: https://github.com/ai-warevo/gitsync/compare/v0.2.0...v0.3.0
[0.2.0]: https://github.com/ai-warevo/gitsync/compare/v0.1.0...v0.2.0
[0.1.0]: https://github.com/ai-warevo/gitsync/releases/tag/v0.1.0
