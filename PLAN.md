# Plan: Create New C++23 gitsync in ai-warevo Organization

## Goal
Create a **new repository** in the ai-warevo organization with a C++23 implementation of gitsync, generated from `template_cpp`, using ai-warevo reusable CI workflows from `~/workspace/.github/.github/{actions,workflows}`.

## Success Criteria
- New repository `ai-warevo/gitsync` with C++23 codebase
- `make build` compiles the project, `make test` passes all tests, `make lint` runs code quality checks
- `cmake --preset clang18-libcxx` and `cmake --preset release` work
- CI workflows (`pr-build.yml`, `pr-test.yml`, `weekly-deep.yml`) use ai-warevo reusable workflows
- `release.yml` builds binaries for Linux and Windows, creates GitHub Release with changelog + artifacts
- Binary outputs: `gitsync` (Linux) / `gitsync.exe` (Windows)
- All Rust business logic successfully ported to C++23

## Prerequisites Analysis
- **Current Rust project**: `/home/node/workspace/gitsync` (15 .rs files)
- **Dependencies**: nlohmann_json v3.11.3, spdlog v1.14.1, Catch2 v3.7.1
- **Config format**: JSON with PascalCase fields (LogLevel, SourceRemoteName, RemoteBranchTemplate, RemoteUrls, SshKeyTypes)
- **Reusable workflows**: Located at `/home/node/workspace/.github/.github/workflows/`
- **Container image**: `ghcr.io/ai-warevo/ci-cpp23`

---

## Implementation Phases

### Phase 1: Repository Setup
**Configure git remotes for ai-warevo organization workflow**

✅ **Completed:**
1. New repository created: `https://github.com/ai-warevo/gitsync`
2. Git remotes configured in `/home/node/workspace/gitsync`:
   - `origin` → `git@github.com:ai-warevo/gitsync.git` (target organization repo)
   - `dshagent` → `git@github.com:ai-vibeqodez/gitsync.git` (agent's fork for PRs)
   - `myrepos` → `git@github.com:my-repositories/gitsync.git` (old Rust version archive)

**Workflow:**
- Agent works on feature branches locally
- Push branches to `dshagent` (ai-vibeqodez/gitsync fork)
- Create PRs from `ai-vibeqodez/gitsync` → `ai-warevo/gitsync`
- After PR merge, pull from `origin` (ai-warevo) to stay in sync

### Phase 2: Archive Rust Source for Reference
**Preserve current implementation**

**Strategy:** Keep current workspace at `/home/node/workspace/gitsync` as the working directory. The Rust source is already preserved in `myrepos` remote and will remain accessible for reference during migration.

**Action:** Before starting C++23 generation, move Rust files to a temporary subdirectory:
```bash
cd /home/node/workspace/gitsync
mkdir -p .rust-archive
mv src/ tests/ Cargo.toml Cargo.lock .rust-archive/
```

This keeps the Rust source in the working tree for easy reference while clearing space for C++ generation.

### Phase 3: Generate C++23 Project Skeleton
**Use template_cpp generator**

Execute from template_cpp directory:
```bash
cd /home/node/workspace/template_cpp
make init TYPE=cli NAME=gitsync NS=aw::gitsync OUT=/home/node/workspace/gitsync
```

**Note:** This will generate directly into the current working repository at `/home/node/workspace/gitsync`, overwriting/creating:
- `CMakeLists.txt`
- `include/aw/gitsync/gitsync.hpp`
- `src/gitsync.cpp`
- `src/main.cpp`
- `tests/gitsync_tests.cpp`
- `tests/CMakeLists.txt`
- `tests/.clang-tidy`

**Copied as-is**:
- `.clang-format`, `.clang-tidy`, `.editorconfig`
- `.gitignore`, `.gitattributes`
- `CMakePresets.json`, `VERSION`

### Phase 4: Configure Dependencies
**Update root CMakeLists.txt**

Add FetchContent dependencies after `project()`:
```cmake
include(FetchContent)

# JSON library
FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json
  GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

# Logging library
FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog
  GIT_TAG v1.14.1
)
FetchContent_MakeAvailable(spdlog)

# Testing framework (tests/CMakeLists.txt)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.7.1
)
FetchContent_MakeAvailable(Catch2)
include(Catch)
```

Link dependencies to gitsync target:
```cmake
target_link_libraries(gitsync PRIVATE nlohmann_json::nlohmann_json spdlog::spdlog)
```

### Phase 5: Extend CMakePresets for Windows
**Add MSVC preset to CMakePresets.json**

Insert new configure preset:
```json
{
  "name": "msvc",
  "displayName": "Release, MSVC (Windows)",
  "condition": {
    "type": "equals",
    "lhs": "${hostSystemName}",
    "rhs": "Windows"
  },
  "inherits": "release",
  "cacheVariables": {
    "CMAKE_CXX_COMPILER": "cl"
  }
}
```

Add corresponding build and test presets referencing `msvc`.

### Phase 6: Create CI Workflows
**Adapt workflows from template_cpp for real project**

#### `pr-build.yml`
```yaml
name: PR Build

on:
  pull_request:
    branches: [main, master]
  merge_group:
    branches: [main, master]

permissions:
  contents: read

jobs:
  fast-checks:
    uses: ai-warevo/.github/.github/workflows/reusable-pr-fast.yml@main
    with:
      source_dir: .
      build_dir: build
      build_type: Debug
      extra_args: -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
      extensions: cpp,hpp,c,h
      container_image: ghcr.io/ai-warevo/ci-cpp23
```

**Changes from template**:
- `source_dir: .ci-project` → `source_dir: .`
- Remove `setup_command` (no generation needed)
- Remove `CI_CHECK_BUILD_TESTS`

#### `pr-test.yml`
```yaml
name: PR Test

on:
  pull_request:
    branches: [main, master]

permissions:
  contents: read

concurrency:
  group: ${{ github.workflow }}-${{ github.ref }}
  cancel-in-progress: true

jobs:
  unit-tests:
    uses: ai-warevo/.github/.github/workflows/reusable-matrix.yml@main
    with:
      source_dir: .
      build_dir: build
      container_image: ghcr.io/ai-warevo/ci-cpp23
      linux_toolchains: '["clang18-libcxx", "gcc15"]'
      build_types: '["Debug"]'
      enable_windows: false
      ctest_timeout: 120
```

**Changes from template**:
- `source_dir: .ci-project` → `source_dir: .`
- Remove `setup_command`
- Remove `CI_CHECK_BUILD_TESTS`

#### `weekly-deep.yml`
```yaml
name: Weekly Deep

on:
  schedule:
    - cron: '0 3 * * 1'
  workflow_dispatch:

permissions:
  contents: read

jobs:
  deep-analysis:
    uses: ai-warevo/.github/.github/workflows/reusable-weekly-deep.yml@main
    with:
      source_dir: .
      build_dir: build
      container_image: ghcr.io/ai-warevo/ci-cpp23
```

**Changes from template**:
- `source_dir: .ci-project` → `source_dir: .`
- Remove `setup_command`

#### `release.yml` — Custom Binary Build + Changelog
```yaml
name: Release

on:
  push:
    tags:
      - 'v*'
  workflow_dispatch:
    inputs:
      version:
        description: 'Version override (X.Y.Z); use auto to derive from CHANGELOG.md'
        required: false
        default: auto
        type: string

permissions:
  contents: write
  actions: write

jobs:
  build:
    strategy:
      matrix:
        include:
          - os: ubuntu-latest
            preset: release
            binary_name: gitsync
            artifact_name: gitsync-linux
          - os: windows-latest
            preset: msvc
            binary_name: gitsync.exe
            artifact_name: gitsync-windows
    runs-on: ${{ matrix.os }}
    
    steps:
      - uses: actions/checkout@v4
      
      - name: Setup CMake
        uses: lukka/get-cmake@latest
      
      - name: Configure
        run: cmake --preset ${{ matrix.preset }}
      
      - name: Build
        run: cmake --build --preset ${{ matrix.preset }}
      
      - name: Upload artifact
        uses: actions/upload-artifact@v4
        with:
          name: ${{ matrix.artifact_name }}
          path: build/${{ matrix.preset }}/${{ matrix.binary_name }}

  release:
    needs: build
    uses: ai-warevo/.github/.github/workflows/reusable-release.yml@main
    with:
      version: ${{ inputs.version || 'auto' }}
      collect_artifacts: true
    permissions:
      contents: write
      actions: write
```

**Key design**:
- Build job produces Linux + Windows binaries
- Release job calls reusable workflow that promotes changelog and creates GitHub Release
- `reusable-release.yml` should collect uploaded artifacts and attach them to the release

### Phase 7: Create Makefile with Lint Target
**Add convenience targets**

Create `Makefile` in project root:
```makefile
.PHONY: build test lint clean help

# Default preset for local development
PRESET ?= dev

help:
	@echo "Available targets:"
	@echo "  make build    - Build project (Debug by default)"
	@echo "  make test     - Run all tests"
	@echo "  make lint     - Run clang-format and clang-tidy checks"
	@echo "  make clean    - Remove build directories"
	@echo ""
	@echo "Environment variables:"
	@echo "  PRESET - CMake preset to use (default: dev)"

build:
	@echo "Building with preset: $(PRESET)"
	cmake --preset $(PRESET)
	cmake --build --preset $(PRESET)

test: build
	@echo "Running tests with preset: $(PRESET)"
	ctest --preset $(PRESET)

lint:
	@echo "Running clang-format check..."
	find src include tests -name '*.cpp' -o -name '*.hpp' | xargs clang-format --dry-run --Werror
	@echo "Running clang-tidy..."
	cmake --preset dev -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
	find src -name '*.cpp' | xargs clang-tidy -p build/dev

clean:
	@echo "Cleaning build directories..."
	rm -rf build
```

**Lint targets explained**:
- **clang-format**: Check formatting without modifying files (`--dry-run --Werror`)
- **clang-tidy**: Static analysis using compile commands database
- Runs on `src/`, `include/`, `tests/` with `.cpp` and `.hpp` extensions

### Phase 8: Port Business Logic to C++23
**Module-by-module migration from Rust**

Reference: `.rust-archive/src/` (Rust source files archived in working directory)

#### 8.1 Configuration Module

**Files to create:**
- `include/aw/gitsync/configuration/config_settings.hpp`
- `include/aw/gitsync/configuration/config_reader.hpp`
- `src/configuration/config_settings.cpp`
- `src/configuration/config_reader.cpp`

**Rust → C++ mapping:**
- `PartialConfigSettings` → `struct PartialConfig` with `std::optional<T>` fields
- `ConfigSettings` → `struct Config` with concrete values
- `merge()` logic → member function or free function
- JSON deserialization → `nlohmann::json::parse()` with manual field extraction (PascalCase)

**Key functionality:**
- Read from `~/.gitsync/config.json`, `../.gitsync/config.json`, `./.gitsync/config.json`
- Merge hierarchy: Global → Organization → Repository
- Support `GITSYNC_CONFIG_PATH` environment override
- Default values: `LogLevel="info"`, `SourceRemoteName="origin"`, `RemoteBranchTemplate="%owner%/%reponame%/%branchname%"`

#### 8.2 Utils Module

**Files to create:**
- `include/aw/gitsync/utils.hpp`
- `src/utils.cpp`

**Functions from Rust `utils.rs`:**
- Version parsing and comparison
- Branch name sanitization (remove special chars, apply template)
- String manipulation helpers
- Path resolution utilities

#### 8.3 Services Module

**Files to create:**
- `include/aw/gitsync/services/process_runner.hpp`
- `include/aw/gitsync/services/known_hosts_service.hpp`
- `include/aw/gitsync/services/git_sync_service.hpp`
- `src/services/process_runner.cpp`
- `src/services/known_hosts_service.cpp`
- `src/services/git_sync_service.cpp`

**ProcessRunner:**
- Execute shell commands (use `std::system()` or `popen()` initially, consider `boost::process` later)
- Capture stdout/stderr
- Return exit code + output
- Cross-platform handling (Unix vs Windows)

**KnownHostsService:**
- Run `ssh-keyscan` for configured SSH key types
- Add hosts to `~/.ssh/known_hosts`
- Parse and validate SSH keys

**GitSyncService:**
- Core sync logic: fetch, push, branch creation
- Execute git commands via ProcessRunner
- Apply RemoteBranchTemplate
- Iterate over RemoteUrls and push to each

#### 8.4 Main Entry Point

**File:** `src/main.cpp`

**CLI actions** (from Rust main.rs):
- Default: run sync
- `-v, --version`: print version
- `-u, --update`: check for updates
- `-h, --help`: print help

**Flow:**
1. Parse CLI args
2. Load config via ConfigReader
3. Initialize logging with spdlog (level from config)
4. Create ProcessRunner, KnownHostsService, GitSyncService
5. Run `known_hosts_service.warm_up()` (unless `--skip-host`)
6. Run `git_sync_service.sync()`
7. Handle errors with detailed messages

### Phase 9: Write Comprehensive Tests
**Catch2 v3 test suite**

#### Test files structure:
- `tests/utils_tests.cpp` — version parsing, branch sanitization
- `tests/config_reader_tests.cpp` — config merge hierarchy, defaults, env override
- `tests/process_runner_tests.cpp` — command execution, output capture
- `tests/git_sync_service_tests.cpp` — git command construction, branch template application
- `tests/known_hosts_service_tests.cpp` — ssh-keyscan execution, key validation

**Test strategy:**
- Mock ProcessRunner for service tests (inject mock, verify commands called)
- Use temporary directories for config file hierarchy tests
- Validate JSON parsing with valid/invalid samples
- Edge cases: missing config files, malformed JSON, empty RemoteUrls

**Configure tests/CMakeLists.txt:**
```cmake
target_link_libraries(gitsync_tests PRIVATE 
  Catch2::Catch2WithMain
  nlohmann_json::nlohmann_json
  spdlog::spdlog
)

catch_discover_tests(gitsync_tests)
```

### Phase 10: Documentation and Metadata
**Port documentation and setup project files**

1. Copy and adapt `.rust-archive/docs/README.md`
   - Update build instructions (Rust → CMake)
   - Keep configuration documentation (JSON format stays the same)

2. Create/update project files:
   - `README.md` — project description, build instructions, usage examples
   - `CHANGELOG.md` — start with v1.0.0 for C++ rewrite
   - `VERSION` — initial version (e.g., `1.0.0`)
   - `LICENSE` — keep existing license from Rust version

### Phase 11: Integration and Verification
**Pre-push checklist**

1. **Local build verification:**
   - `cmake --preset clang18-libcxx` — Debug build
   - `cmake --preset release` — Release build
   - `cmake --preset msvc` — Windows build (if on Windows, or skip)
   - `make build && make test` — full cycle
   - `make lint` — code quality checks

2. **Functional testing:**
   - Create test config in `.gitsync/config.json`
   - Run binary: `./build/release/gitsync --help`
   - Run binary: `./build/release/gitsync --version`
   - Test sync with dummy remote URLs (dry-run or test repos)

3. **Code quality:**
   - Run `make lint` to verify formatting and static analysis
   - Fix any issues reported

4. **Push to ai-warevo/gitsync:**
   - Push main branch
   - Verify PR Build workflow runs successfully
   - Verify PR Test workflow runs successfully

5. **Create initial release:**
   - Tag: `v1.0.0`
   - Push tag
   - Verify release workflow builds binaries
   - Verify GitHub Release created with changelog + artifacts (gitsync, gitsync.exe)

---

## Final Structure

```
ai-warevo/gitsync/
├── CMakeLists.txt
├── CMakePresets.json
├── Makefile (with build, test, lint, clean targets)
├── VERSION
├── README.md
├── CHANGELOG.md
├── LICENSE
├── .clang-format
├── .clang-tidy
├── .editorconfig
├── .gitignore
├── .gitattributes
├── docs/
│   └── README.md (configuration guide)
├── src/
│   ├── main.cpp
│   ├── gitsync.cpp
│   ├── utils.cpp
│   ├── configuration/
│   │   ├── config_reader.cpp
│   │   └── config_settings.cpp
│   └── services/
│       ├── git_sync_service.cpp
│       ├── known_hosts_service.cpp
│       └── process_runner.cpp
├── include/
│   └── aw/
│       └── gitsync/
│           ├── gitsync.hpp
│           ├── utils.hpp
│           ├── configuration/
│           │   ├── config_reader.hpp
│           │   └── config_settings.hpp
│           └── services/
│               ├── git_sync_service.hpp
│               ├── known_hosts_service.hpp
│               └── process_runner.hpp
├── tests/
│   ├── CMakeLists.txt
│   ├── .clang-tidy
│   ├── gitsync_tests.cpp (generated placeholder)
│   ├── utils_tests.cpp
│   ├── config_reader_tests.cpp
│   ├── process_runner_tests.cpp
│   ├── git_sync_service_tests.cpp
│   └── known_hosts_service_tests.cpp
└── .github/
    └── workflows/
        ├── pr-build.yml
        ├── pr-test.yml
        ├── weekly-deep.yml
        └── release.yml
```

---

## Assumptions

1. `ai-warevo/.github` reusable workflows (`reusable-pr-fast.yml`, `reusable-matrix.yml`, `reusable-weekly-deep.yml`, `reusable-release.yml`) support the parameters specified above
2. `reusable-release.yml` can collect build artifacts and attach them to GitHub Release
3. Container image `ghcr.io/ai-warevo/ci-cpp23` has clang-18, gcc-15, libc++, CMake ≥3.25, clang-format, clang-tidy
4. GitHub Actions has write access to create releases in ai-warevo org
5. The current Rust implementation's business logic is complete and correct (no missing features to discover during port)
6. ProcessRunner can use standard library or simple `popen()` initially (no Boost dependency required yet)

---

## Edge Cases and Risks

### Configuration Merging
- **Risk**: Complex hierarchy logic (3 layers + env override)
- **Mitigation**: Write comprehensive tests for each merge scenario; validate against Rust implementation behavior

### Cross-Platform ProcessRunner
- **Risk**: Different behavior of shell commands on Windows vs Linux
- **Mitigation**: Abstract command execution; use CMake platform detection; test on both platforms via CI

### Git Command Execution
- **Risk**: Git output parsing differences across versions
- **Mitigation**: Use porcelain commands where possible; version-check git in CI

### SSH Key Scanning
- **Risk**: `ssh-keyscan` timeout or missing on some systems
- **Mitigation**: Make timeouts configurable; provide clear error messages; document SSH requirements

### Release Artifact Collection
- **Risk**: `reusable-release.yml` might not support artifact collection
- **Mitigation**: Verify workflow capabilities first; if needed, inline artifact collection in release.yml before calling reusable workflow

### Lint in CI
- **Risk**: Clang-format/clang-tidy versions differ between local and CI
- **Mitigation**: Document required versions; use container image versions as source of truth

---

## Open Questions

1. **Changelog format**: Does `reusable-release.yml` expect a specific CHANGELOG.md format (e.g., Keep a Changelog)?
   - **Action**: Check `~/workspace/.github/.github/actions/changelog-*` for format expectations

2. **Version source**: Should VERSION file be authoritative, or derived from git tags?
   - **Action**: Align with ai-warevo convention (check template_cpp or other repos)

3. **Artifact paths in release build**: Does CMake output to `build/<preset>/` or `build/`?
   - **Action**: Test locally or check CMakePresets.json `binaryDir` setting

4. **ProcessRunner implementation**: Should we use Boost.Process for production quality, or is `popen()` sufficient?
   - **Action**: Start with simple implementation; refactor if needed after testing

---

## Success Metrics

- ✅ All CI workflows pass on first push
- ✅ `make lint` passes with no warnings
- ✅ Release workflow produces both Linux and Windows binaries
- ✅ All Rust tests ported and passing in C++
- ✅ Functional test: successfully sync a real repository using the C++ binary
- ✅ Documentation covers build, configuration, and usage
- ✅ Code coverage ≥80% for core business logic
