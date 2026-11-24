# Public API Stabilization for gitsync v1.0.0

**Status:** Complete  
**Version:** 1.0.0  
**Namespace:** `aw::gitsync`

---

## Overview

gitsync v1.0.0 establishes a formal public API contract for library consumers. These APIs are guaranteed stable within the v1.x release series. Breaking changes are deferred to v2.0.0+.

---

## Stable Public APIs (Guaranteed v1.0 → v1.x)

### 1. ConfigSettings (Data Container)

**Location:** `include/aw/gitsync/config_settings.hpp`

```cpp
namespace aw::gitsync {
    struct ConfigSettings {
        std::string log_level;
        std::string remote_branch_template;
        std::map<std::string, std::string> remote_urls;
        std::string source_remote_name;
        std::string ssh_key_types;
        int max_retries;
        std::vector<std::string> include_branches;
        std::vector<std::string> exclude_branches;
        std::vector<std::string> include_remotes;
        std::vector<std::string> exclude_remotes;
    };
}
```

**Stability Guarantee:** Data members are stable. Order may change in v2.0+, but names and types are fixed.

**Usage:**
```cpp
auto settings = ConfigReader::read_config();
std::cout << settings.log_level;  // "info", "debug", etc.
std::cout << settings.max_retries;  // e.g., 1, 2, 3
```

---

### 2. PartialConfigSettings (Merge Container)

**Location:** `include/aw/gitsync/config_settings.hpp`

```cpp
struct PartialConfigSettings {
    std::optional<std::string> log_level;
    std::optional<std::string> remote_branch_template;
    std::optional<std::map<std::string, std::string>> remote_urls;
    std::optional<std::string> source_remote_name;
    std::optional<std::string> ssh_key_types;
    std::optional<int> max_retries;
    std::optional<std::vector<std::string>> include_branches;
    std::optional<std::vector<std::string>> exclude_branches;
    std::optional<std::vector<std::string>> include_remotes;
    std::optional<std::vector<std::string>> exclude_remotes;

    void merge(const PartialConfigSettings& local);
    [[nodiscard]] ConfigSettings into_final_settings() const;
};
```

**Stability Guarantee:** Methods and types are stable.

**Usage:**
```cpp
PartialConfigSettings partial;
partial.log_level = "debug";
partial.merge(other_partial);
auto final = partial.into_final_settings();
```

---

### 3. ConfigReader (Configuration Loading)

**Location:** `include/aw/gitsync/config_reader.hpp`

```cpp
class ConfigReader {
public:
    static ConfigSettings read_config();
    static PartialConfigSettings read_config_from_path(
        const std::filesystem::path& config_path);
    static std::vector<std::filesystem::path> collect_config_paths();
    static std::vector<std::filesystem::path> resolve_config_hierarchy(
        const std::optional<std::string>& config_env,
        const std::optional<std::filesystem::path>& home_dir,
        const std::optional<std::filesystem::path>& current_dir);
};
```

**Stability Guarantee:** Method signatures are stable. Implementation may improve.

**Usage:**
```cpp
// Load all three config layers (global → org → repo)
auto settings = ConfigReader::read_config();

// Load single config file (JSON or YAML)
auto partial = ConfigReader::read_config_from_path("~/.gitsync/config.yaml");

// Discover config paths following hierarchy
auto paths = ConfigReader::collect_config_paths();
```

**Supported Formats:**
- JSON: `config.json`
- YAML: `config.yaml` or `config.yml`
- Dual naming convention: Both PascalCase and snake_case field names

---

### 4. GitSyncService (Synchronization)

**Location:** `include/aw/gitsync/git_sync_service.hpp`

```cpp
class GitSyncService {
public:
    explicit GitSyncService(const ConfigSettings& settings);
    void perform_sync();
    
    // Utility functions (stable)
    [[nodiscard]] static std::string sanitize_branch_name(std::string_view input);
    [[nodiscard]] static std::string build_remote_branch(
        std::string_view template_str,
        std::string_view git_url,
        std::string_view branch_name);
};
```

**Stability Guarantee:** Public methods are stable. Internal implementation may optimize.

**Usage:**
```cpp
auto settings = ConfigReader::read_config();
GitSyncService service(settings);
service.perform_sync();

// Utility methods
auto sanitized = GitSyncService::sanitize_branch_name("feature/test");
auto branch = GitSyncService::build_remote_branch(
    "%owner%/%reponame%/%branchname%",
    "git@github.com:alice/project.git",
    "main");
```

---

## Internal APIs (May Change Without Notice)

**Do NOT use these in production code:**

- `KnownHostsService` — Internal SSH host key verification utility
- `ProcessRunner` — Internal process execution wrapper
- `utils.hpp` — Internal helper functions
- Private methods of stable classes

---

## Configuration File Format (v1.0+)

### Supported Naming Conventions

Both **PascalCase** (v0.6.0 legacy) and **snake_case** (v1.0+ recommended) are supported:

| Field | PascalCase | snake_case | Type | Default |
|-------|-----------|-----------|------|---------|
| Log Level | `LogLevel` | `log_level` | string | `"info"` |
| Source Remote | `SourceRemoteName` | `source_remote_name` | string | `"origin"` |
| Branch Template | `RemoteBranchTemplate` | `remote_branch_template` | string | `"%owner%/%reponame%/%branchname%"` |
| Remote URLs | `RemoteUrls` | `remote_urls` | map | `{}` |
| SSH Key Types | `SshKeyTypes` | `ssh_key_types` | string | `""` |
| Max Retries | `MaxRetries` | `max_retries` | int | `1` |
| Include Branches | `IncludeBranches` | `include_branches` | array | `["*"]` |
| Exclude Branches | `ExcludeBranches` | `exclude_branches` | array | `[]` |
| Include Remotes | `IncludeRemotes` | `include_remotes` | array | `[]` |
| Exclude Remotes | `ExcludeRemotes` | `exclude_remotes` | array | `["upstream", "base"]` |

**Precedence Rule:** snake_case takes precedence if both names present in same config file.

### JSON Example

```json
{
  "log_level": "info",
  "source_remote_name": "origin",
  "remote_branch_template": "%owner%/%reponame%/%branchname%",
  "remote_urls": {
    "m1": "https://github.com/alice/project.git",
    "m2": "git@gitlab.com:bob/project.git"
  },
  "max_retries": 2,
  "include_branches": ["main", "release/*"],
  "exclude_remotes": ["upstream", "base"]
}
```

### YAML Example

```yaml
log_level: info
source_remote_name: origin
remote_branch_template: "%owner%/%reponame%/%branchname%"
remote_urls:
  m1: https://github.com/alice/project.git
  m2: git@gitlab.com:bob/project.git
max_retries: 2
include_branches:
  - main
  - release/*
exclude_remotes:
  - upstream
  - base
```

---

## Configuration Hierarchy

Configurations are merged in order (later layers override earlier):

1. **Global:** `~/.gitsync/config.json` or `~/.gitsync/config.yaml`
2. **Organization:** `../.gitsync/config.json` or `../.gitsync/config.yaml` (parent directory)
3. **Repository:** `./.gitsync/config.json` or `./.gitsync/config.yaml` (current directory)
4. **Override:** `GITSYNC_CONFIG_PATH` environment variable (takes absolute priority)

**File Format Selection:** Per-layer, e.g., global can be JSON while repo is YAML.

---

## Exit Codes (Stable v1.0+)

| Code | Meaning | Action |
|------|---------|--------|
| **0** | Success | All remotes synced successfully |
| **1** | General error | Unhandled exception, check logs |
| **2** | Partial success | N of M remotes synced, review warnings |
| **3** | Config error | Invalid/missing config, fix and retry |
| **4** | Git error | Not a repo or branch mismatch, manual fix needed |
| **5** | Permission error | SSH key/file permissions denied, fix credentials |

---

## Versioning Policy (Stable v1.0+)

### v1.0.0 → v1.x (Stable)
- **Guaranteed:** No breaking changes to public APIs
- **Allowed:** New optional fields in ConfigSettings
- **Allowed:** New optional CLI flags
- **Allowed:** New methods in public classes (additive)
- **Allowed:** Performance and bug fixes

### v2.0.0+ (Future)
- **May break:** Deprecated fields/methods removed
- **May break:** Namespace reorganization
- **May break:** Configuration format changes

### Deprecation Path
When an API is planned for removal:
1. Mark with `[[deprecated("reason")]]`
2. Document alternative in release notes
3. Provide 2+ minor releases (e.g., v1.1, v1.2) before removal
4. Remove only in major version (v2.0.0+)

---

## Recommended Usage for Library Consumers

### Installation
```cpp
#include <aw/gitsync/config_settings.hpp>
#include <aw/gitsync/config_reader.hpp>
#include <aw/gitsync/git_sync_service.hpp>
```

### Or use the public API header
```cpp
#include <aw/gitsync/public_api.hpp>
```

### Basic Pattern
```cpp
#include <aw/gitsync/config_settings.hpp>
#include <aw/gitsync/config_reader.hpp>
#include <aw/gitsync/git_sync_service.hpp>
#include <spdlog/spdlog.h>
#include <iostream>

int main() {
    try {
        // Load configuration
        auto settings = aw::gitsync::ConfigReader::read_config();
        
        // Create service
        aw::gitsync::GitSyncService service(settings);
        
        // Perform sync
        service.perform_sync();
        
        spdlog::info("Sync completed successfully");
        return 0;
    } catch (const std::exception& e) {
        spdlog::error("Sync failed: {}", e.what());
        return 1;
    }
}
```

---

## Stability Guarantee Summary

| Component | v1.0 | v1.x | v2.0+ |
|-----------|------|------|-------|
| ConfigSettings | ✅ Stable | ✅ Stable | ⚠️ May change |
| PartialConfigSettings | ✅ Stable | ✅ Stable | ⚠️ May change |
| ConfigReader | ✅ Stable | ✅ Stable | ⚠️ May change |
| GitSyncService | ✅ Stable | ✅ Stable | ⚠️ May change |
| KnownHostsService | ❌ Internal | ❌ Internal | — |
| ProcessRunner | ❌ Internal | ❌ Internal | — |
| Exit codes | ✅ Stable | ✅ Stable | ⚠️ May change |
| Config file formats | ✅ Stable | ✅ Stable | ⚠️ May change |

---

## Migration from v0.6.0 to v1.0.0

**Good news:** No migration required!

- v1.0.0 accepts both PascalCase (v0.6.0) and snake_case (v1.0.0) field names
- Existing configs continue to work unchanged
- Gradual migration to snake_case at your own pace
- No breaking changes within v1.x

**Example:** This v0.6.0 config works unchanged in v1.0.0:
```json
{
  "LogLevel": "info",
  "SourceRemoteName": "origin",
  "RemoteUrls": { "m1": "https://..." }
}
```

You can optionally migrate to snake_case:
```json
{
  "log_level": "info",
  "source_remote_name": "origin",
  "remote_urls": { "m1": "https://..." }
}
```

Both work in v1.0.0 and v1.x. Choose at your discretion.

---

## Questions or Issues?

If you discover a breaking change in v1.x, please report it as a bug. Stability is guaranteed within v1.0 → v1.x.
