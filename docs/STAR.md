# Case Study: Cross-Platform CLI Utility GitSync Development (STAR Methodology)

## 📌 Situation (Situation)
When working with git repositories within large organizations or personal backups, there is a recurring need to mirror code (push the same branch) to multiple independent platforms simultaneously (e.g., to a corporate GitLab, a mirror on GitVerse, and a backup on GitHub).

Existing solutions required either complex manual configuration of multiple remote addresses for each local repository, or relied on heavy CI/CD pipelines that slowed down development. Additionally, when working over SSH on new machines, regular build failures occurred due to target servers being absent from the `known_hosts` file.

## 🎯 Task (Task)
Create a lightweight, secure, and compilable cross-platform CLI utility in Rust (`gitsync`) that:
1. Automates SSH session warmup (`ssh-keyscan`) for target hosts.
2. Automatically detects the context of the current git repository and active branch.
3. Supports a flexible, multi-level configuration system (global settings, organization/client-wide settings, and local project settings) without data duplication.
4. Works reliably in both Linux and Windows environments without requiring external dependencies.

## 🎬 Action (Actions)
The following engineering steps were taken during the design and implementation of the Rust-based utility architecture:

1. **Cross-platform path resolution:** Integrated the `dirs` crate for native detection of user home directories regardless of OS (`HOME` on Linux/macOS and `USERPROFILE` on Windows).
2. **Multi-level configuration architecture:** Designed and implemented a cascading configuration file search system (`.gitsync/config.json`) with the following hierarchy:
   * Global user profile (`~/.gitsync/`)
   * Parent organization directory profile (`../.gitsync/`)
   * Local repository profile (`./.gitsync/`)
3. **DTO pattern for parsing and layer merging:** To prevent parser crashes on partially filled files, the logic was split into two structures:
   * Intermediate `PartialConfigSettings` (all fields wrapped in `Option<T>` for flexible Serde deserialization) with a custom `.merge()` method, where local settings supplement `RemoteUrls` maps and precisely override global strings.
   * Final `ConfigSettings` (strict types without `Option`), assembled by the `.into_final_settings()` method with safe system defaults applied.
4. **Testing and automation:**
   * Written unit and integration tests using the `tempfile` crate for file operation isolation and verification of correct config layering in hierarchical folder structures.
   * CI pipeline configured (GitHub Actions) including strict code formatting checks (`cargo fmt`), static analysis (`cargo clippy -- -D warnings`), and automatic binary builds for Linux (`gnu`) and Windows (`msvc`).

## 🏆 Result (Result)
1. **Successful cross-platform execution:** The utility works with a single command (`gitsync`) on both Linux and Windows (thanks to executable integration into the `WindowsApps` system layer) without manual `PATH` variable modification.
2. **Elimination of config duplication:** The "Stack" scenario allows writing base parameters once at the user root, company mirror templates in the organization folder, and leaving only 5 lines of code with mirror links in repositories.
3. **Fault tolerance:** The configuration parser no longer crashes on missing fields, and the application is guaranteed to build and test automatically via CI with zero compiler warnings.
