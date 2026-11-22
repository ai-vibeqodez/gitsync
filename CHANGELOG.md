# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [0.3.0] - 2026-11-22

### Added

- add multi-platform binary builds to release workflow
- rewrite gitsync from Rust to C++23

### Changed

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

- Windows compatibility - use _popen/_pclose and safe getenv
- use ilammy/msvc-dev-cmd and cmake-build action for Windows

## [0.2.0] - 2026-11-22

### Added

- rewrite gitsync from Rust to C++23

### Changed

- fix
- use aw_add_library instead of aw_core_library
- replace vendored dependencies with FetchContent
- update to new workflow structure
- add .clang-tidy and .editorconfig config files
- remove unnecessary documentation files
- add final project completion summary
- use C++23 instead of Rust

## [0.1.0] - 2024-XX-XX (Rust)

### Added
- Initial Rust implementation
- Multi-layer configuration system
- SSH known_hosts warm-up
- Git repository synchronization across multiple remotes
- Template-based branch naming
- CLI with version, help, update, and skip-host options

[Unreleased]: https://github.com/ai-warevo/gitsync/compare/v0.3.0...HEAD
[0.3.0]: https://github.com/ai-warevo/gitsync/compare/v1.1.10...v0.3.0
[0.2.0]: https://github.com/ai-warevo/gitsync/compare/v1.1.10...v0.2.0
[0.1.0]: https://github.com/ai-warevo/gitsync/releases/tag/v0.1.0
