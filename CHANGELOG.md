# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Changed
- Complete rewrite from Rust to C++23
- Build system migrated from Cargo to CMake with presets
- Dependencies now managed via CMake FetchContent
- CI/CD workflows updated to use reusable workflows from ai-warevo/.github

### Added
- CMake presets for clang18-libcxx, release, and msvc builds
- Makefile CI contract with ci-fast, ci-deep, ci-coverage, and build-release targets
- Comprehensive test suite using Catch2 v3
- Static analysis via clang-tidy
- Code formatting enforcement via clang-format
- Coverage badge generation
- Multi-platform release builds (Linux + Windows)

### Maintained
- Full configuration compatibility with Rust version
- Three-layer config merge hierarchy (global → org → repo)
- Environment variable override (GITSYNC_CONFIG_PATH)
- JSON configuration format with PascalCase field names
- SSH known_hosts management
- Multi-remote synchronization logic
- CLI argument compatibility (-v, -h, -u, -s)

## [0.1.0] - 2024-XX-XX (Rust)

### Added
- Initial Rust implementation
- Multi-layer configuration system
- SSH known_hosts warm-up
- Git repository synchronization across multiple remotes
- Template-based branch naming
- CLI with version, help, update, and skip-host options

[Unreleased]: https://github.com/ai-warevo/gitsync/compare/v0.1.0...HEAD
[0.1.0]: https://github.com/ai-warevo/gitsync/releases/tag/v0.1.0
