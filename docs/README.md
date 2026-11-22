# gitsync Configuration

`gitsync` supports a multi-level configuration merge system. This allows flexible configuration of the utility either globally for the entire machine or isolated for individual organizations or specific repositories.

---

## 🛠️ Configuration File Structure

The config file must be named `config.json` and placed inside the `.gitsync` folder.
All fields are **optional**. If a field is omitted at the local level, `gitsync` will take its value from a more global level or apply a default value.

### Available fields and their defaults:
* `LogLevel` — logging level (default: `"info"`).
* `SourceRemoteName` — name of the source git repository (default: `"origin"`).
* `RemoteBranchTemplate` — branch name template for mirroring (default: `"%owner%/%reponame%/%branchname%"`).
* `RemoteUrls` — list (JSON object) of target repositories for synchronization (mirrors).
* `SshKeyTypes` — SSH key types to scan via `ssh-keyscan`. Helps avoid timeouts on Windows if a specific cipher is specified (e.g., `"ed25519"`). If not set, all default types are queried.

---

## 🔝 Priority and Layer Hierarchy

If multiple configuration files are available for a single project, they are layered on top of each other in a chain:

```text
[ 1. Global ] -> [ 2. Organization ] -> [ 3. Repository ]
```
*Each subsequent level supplements or overrides the settings of the previous one.*

---

## 📋 Use Cases

### Scenario 1. Full Stack (Home + Organization + Repository)
The most flexible scenario, currently used for the `ai-warevo` project.

**1. Home config (`~/.gitsync/config.json`):**
Sets general base settings for the entire machine:
```json
{
  "LogLevel": "info",
  "SourceRemoteName": "origin",
  "RemoteBranchTemplate": "%owner%/%reponame%/%branchname%"
}
```

**2. Organization config (`../.gitsync/config.json`):**
Located in the parent folder (e.g., `~/Documents/ai-warevo/.gitsync/config.json`). Sets shared mirrors for all projects of this team/organization:
```json
{
  "RemoteUrls": {
    "gitlab": "git@gitlab.com:loktionov129/gitsynced-ai-warevo.git",
    "gitverse": "git@gitverse.ru:loktionov129/gitsynced-ai-warevo.git"
  }
}
```

**3. Repository config (`./.gitsync/config.json`):**
Located directly inside a specific repository's folder. Allows precise override of settings or adding a unique mirror only for this project:
```json
{
  "LogLevel": "Debug",
  "RemoteUrls": {
    "custom_mirror": "git@github.com:loktionov129/special-mirror.git"
  }
}
```

**Merged result:**
```json
{
  "LogLevel": "Debug",
  "SourceRemoteName": "origin",
  "RemoteBranchTemplate": "%owner%/%reponame%/%branchname%",
  "RemoteUrls": {
    "gitlab": "git@gitlab.com:loktionov129/gitsynced-ai-warevo.git",
    "gitverse": "git@gitverse.ru:loktionov129/gitsynced-ai-warevo.git",
    "custom_mirror": "git@github.com:loktionov129/special-mirror.git"
  }
}
```

---

### Scenario 2. Home Config Only
Ideal if you work alone and mirror all your projects to the same places (e.g., always backing up all personal data to a personal GitLab).

**Home config (`~/.gitsync/config.json`):**
```json
{
  "LogLevel": "info",
  "SourceRemoteName": "origin",
  "RemoteUrls": {
    "backup": "git@gitlab.com:my-personal-account/backup-mirror.git"
  }
}
```
*The utility will use this single file for any runs from any folders on the computer.*

---

### Scenario 3. Home Config + Organization Config
Convenient if you have a global base configured at home and projects are organized into client/company folders (e.g., `~/Projects/CompanyA/` and `~/Projects/CompanyB/`).

* **Home config:** stores `LogLevel` and `SourceRemoteName`.
* **`CompanyA` folder config (`CompanyA/.gitsync/config.json`):** contains company-specific `RemoteUrls`.

When run from any subproject inside `CompanyA`, `gitsync` automatically pulls the home base and adds this company's mirrors.

---

### Scenario 4. Organization Config Only
If there is no home config, but projects are stored in structured folders.

**Organization config (`../.gitsync/config.json`):**
```json
{
  "SourceRemoteName": "upstream",
  "RemoteUrls": {
    "corp_mirror": "git@://company.com:mirror.git"
  }
}
```
* **Result:** `LogLevel` and `RemoteBranchTemplate` will take default values (defaults from the utility's code), while the source name and mirrors will be pulled from the parent folder.

---

### Scenario 5. Repository Config Only
If you need to configure isolated synchronization for a single specific project without cluttering the system's global folders.

Simply create `./.gitsync/config.json` directly in your repository root:
```json
{
  "RemoteUrls": {
    "github_mirror": "git@github.com:user/project.git"
  }
}
```
* **Result:** All settings except `RemoteUrls` will be automatically filled with built-in defaults. No external files (home or parent) will be considered due to their absence.

---

### Scenario 6. Override via Environment Variable (ENV)
This scenario has **absolute priority** and completely disables the entire file hierarchy (the "stack"). It is used for automation, CI/CD scripts, or emergency runs.

You can explicitly specify the path to a specific configuration file via the `GITSYNC_CONFIG_PATH` variable.

#### How to run in PowerShell (Windows):
```powershell
$env:GITSYNC_CONFIG_PATH = "C:\(custom\path\to\special_config.)json"
gitsync
```

#### How to run in Bash (Linux / macOS):
```bash
export GITSYNC_CONFIG_PATH="/custom/path/to/special_config.json"
gitsync
```
*With this launch method, `gitsync` will read **only** the specified `special_config.json` file, apply its settings on top of built-in defaults, and ignore home, organization, and local repository folders.*
