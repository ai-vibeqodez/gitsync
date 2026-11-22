# AGENTS.md

Rules for working with gitsync to prevent common mistakes.

## Before making changes

- **NO PUSH without explicit permission.** Only commit locally and ask before pushing.
- **NO TAG CREATION.** CI creates all tags. Never create tags manually.
- **Ask before changing workflows.** Release workflow is fragile; discuss changes first.

## Git workflow

- Commit locally with clear messages (Conventional Commits: `feat:`, `fix:`, `docs:`, etc.)
- Stage only relevant files: `git add <file>` (never `git add .`)
- Do NOT push to `origin` (ai-warevo). Push to `dshagent` (fork) or ask first.
- Create tags only when CI instructs, never manually.

## CMake and builds

- **Always test locally** before claiming it works:
  - Linux: `cmake --preset clang18-libcxx && cmake --build build/clang18-libcxx && ctest --preset clang18-libcxx`
  - Windows: Use `release.yml` as reference (MSVC + ilammy/msvc-dev-cmd)
- **Release builds skip tests:** `-DGITSYNC_BUILD_TESTS=OFF` to speed up builds
- **FetchContent for dependencies.** Never vendored code. CMake modules from template_cpp via FetchContent.
- **Use correct cmake-build action parameters:** `source_dir`, `build_dir`, `build_type`, `config`, `extra_args`, `parallel`, `install` (no `build_preset`).

## Windows compatibility

- Replace `popen`/`pclose` with `_popen`/`_pclose` using platform macros
- Replace `getenv` with safe `_dupenv_s` on Windows
- HOME environment variable: use `$HOME` on Unix, `%USERPROFILE%` on Windows
- Test both platforms before merging (CI will catch it, but test locally first)

## Release workflow

- **CHANGELOG.md is the source of truth.** Release Notes come from it.
- **Update CHANGELOG under `## [Unreleased]`** for each change:
  - `### Added` — new features
  - `### Changed` — refactoring, build system changes
  - `### Fixed` — bug fixes
- **When releasing:**
  1. Update CHANGELOG: rename `## [Unreleased]` to `## [X.Y.Z] - YYYY-MM-DD`
  2. Commit: `git commit -m "docs: update CHANGELOG for vX.Y.Z"`
  3. Wait for CI or ask — CI creates the tag and builds binaries
- **Release Notes generation:**
  - `what-included` and `quick-start` defaults are overridden to empty strings (no boilerplate)
  - Only CHANGELOG content appears in GitHub Release

## Common mistakes to avoid

1. **Pushing without permission** — Ask first
2. **Creating tags manually** — CI does this
3. **Testing only on one platform** — Test Linux + Windows
4. **Forgetting to update CHANGELOG** — It's required for releases
5. **Using `build_preset` in cmake-build action** — Use `build_type` and `config` instead
6. **Copying boilerplate from ai-warevo/.github examples** — Customize for gitsync (remove defaults)
7. **Assembling test artifacts in release** — Use `-DGITSYNC_BUILD_TESTS=OFF`

## File locations and responsibilities

- `CMakeLists.txt` — Root CMake config (uses FetchContent for aw_cmake_modules)
- `CMakePresets.json` — Build presets (copied from templates/_common/files/, same across all projects)
- `.github/workflows/release.yml` — Build binaries + publish release (customize `what-included`/`quick-start`)
- `CHANGELOG.md` — Release notes source (required format: `## [X.Y.Z] - YYYY-MM-DD`)
- `AGENTS.md` — This file (project-specific rules)
- `src/` — Implementation files (all platform-safe)

## Testing

```bash
# Configure
cmake --preset clang18-libcxx

# Build
cmake --build build/clang18-libcxx

# Test all 11 tests
ctest --preset clang18-libcxx --output-on-failure
```

Tests cover:
- Configuration parsing and merging
- Branch name templating
- SSH URL parsing
- Known hosts line validation

## Cross-platform gotchas

- **POSIX vs Windows paths:** Use `std::filesystem::path` (platform-agnostic)
- **Environment variables:** Always use the safe wrapper or platform macros
- **Process execution:** Wrapped with platform-specific macros in process_runner.cpp
- **Known hosts location:** `~/.ssh/known_hosts` on Unix → `%USERPROFILE%\.ssh\known_hosts` on Windows

## Documentation

- Update README.md if behavior changes
- Keep this file (AGENTS.md) up-to-date with new rules
- Link to code examples when rules are complex
- Comment why, not what (code shows what; comments explain intent)
