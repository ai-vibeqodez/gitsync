# 🔄 GitSync

`GitSync` is a lightweight CLI utility for synchronizing the current git branch across multiple remote repositories and preparing SSH `known_hosts`.

[![CI](https://github.com/my-repositories/gitsync/actions/workflows/ci.yml/badge.svg)](https://github.com/my-repositories/gitsync/actions/workflows/ci.yml)
[![Coverage](https://raw.githubusercontent.com/my-repositories/gitsync/refs/heads/badges/coverage.svg)](https://my-repositories.github.io/gitsync)

## 🛠️ How It Works

When run normally, the application sequentially performs the following steps:

1. Searches for and merges (merges) all available configuration files.
2. Verifies that the current directory is inside a git repository.
3. Updates `known_hosts` via `ssh-keyscan` (unless the `--skip-host` flag is provided).
4. Retrieves the current git branch name and builds the target `refspec`.
5. Automatically adds or updates remote URLs in the repository settings.
6. Pushes the current branch to all target remotes defined in the configuration.
7. Outputs a final summary of successful and failed operations.

---

## 📚 Documentation and Use Cases

For a deeper dive into the project, the following resources are available:

* 📘 **[Configuration Guide (docs/README.md)](docs/README.md)** — detailed examples of file structure, merge logic, and real-world usage scenarios.
* ⭐️ **[Architecture Case Study (docs/STAR.md)](docs/STAR.md)** — project breakdown using the **STAR** methodology (Situation, Task, Action, Result). Description of engineering challenges, the DTO pattern for merging layers, and cross-platform solutions in Rust.

---

## 📦 GitHub Releases

Prebuilt binaries are published automatically when a tag of the form `v*` is created.

Available platforms:
- Linux `x86_64-unknown-linux-gnu`
- Windows `x86_64-pc-windows-msvc`

---

## 📥 Installation

[Download](https://github.com/my-repositories/gitsync/releases/latest) the latest release for your platform from **GitHub Releases** and run `gitsync`.

### Linux

```bash
./gitsync
```

### Windows

You can place the binary in the built-in `%LOCALAPPDATA%\Microsoft\WindowsApps` folder so the utility is globally available in any terminal:

```powershell
gitsync
```

---

## ⚙️ Configuration

The application supports **multi-level configuration merging**. You can split configuration into parts: store global settings at home, shared mirrors in the organization folder, and unique backups at the project root.

By default, `gitsync` automatically collects and layers configuration files in the following chain (local overrides global):

1. **Global level:** `~/.gitsync/config.json`
2. **Organization level (parent):** `../.gitsync/config.json`
3. **Project level (local):** `./.gitsync/config.json`

> 📘 **Detailed Guide**
> For detailed examples of file structure, parameter merge logic, and ready-made usage scenarios, see **[Configuration Documentation (docs/README.md)](docs/README.md)**.

### Environment Variable (Forced Config)
If the `GITSYNC_CONFIG_PATH` environment variable is set, the folder hierarchy above is completely ignored, and the utility reads **only the specified file**:
* **Linux:** `export GITSYNC_CONFIG_PATH=~/my-config.json`
* **Windows (PowerShell):** `$env:GITSYNC_CONFIG_PATH="C:\my-config.json"`

---

## 🚀 Usage

Normal run (the utility automatically builds the configuration stack from folders):

```bash
gitsync
```

Run with forced specification of a single config file:

* **Linux:** `GITSYNC_CONFIG_PATH=~/my-config.json gitsync`
* **Windows (PowerShell):** `$env:GITSYNC_CONFIG_PATH="C:\my-config.json"; gitsync`

Run without updating `known_hosts`:

```bash
gitsync --skip-host
```

## Flags

- `--skip-host` — skip `known_hosts` warmup.

---

## ⚠️ Common Issues

### Config Not Found

If no `.gitsync/config.json` file is found in the path hierarchy (and no ENV variable is set), the application will exit with an error.

### Repository Not Detected

If the command is run outside a git repository, the application will exit with a message stating that the current directory is not a git repository.

### SSH Issues

If `ssh-keyscan` is unavailable or the host does not respond, `known_hosts` warmup may fail.
