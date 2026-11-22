#include "aw/gitsync/config_reader.hpp"

#include <cstdlib>
#include <fstream>
#include <stdexcept>

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

using json = nlohmann::json;

namespace aw::gitsync {

PartialConfigSettings from_json(const json& j) {
    PartialConfigSettings result;

    if (j.contains("LogLevel") && j["LogLevel"].is_string()) {
        result.log_level = j["LogLevel"].get<std::string>();
    }
    if (j.contains("RemoteBranchTemplate") && j["RemoteBranchTemplate"].is_string()) {
        result.remote_branch_template = j["RemoteBranchTemplate"].get<std::string>();
    }
    if (j.contains("SourceRemoteName") && j["SourceRemoteName"].is_string()) {
        result.source_remote_name = j["SourceRemoteName"].get<std::string>();
    }
    if (j.contains("SshKeyTypes") && j["SshKeyTypes"].is_string()) {
        result.ssh_key_types = j["SshKeyTypes"].get<std::string>();
    }
    if (j.contains("RemoteUrls") && j["RemoteUrls"].is_object()) {
        std::map<std::string, std::string> urls;
        for (const auto& [key, val] : j["RemoteUrls"].items()) {
            if (val.is_string()) {
                urls[key] = val.get<std::string>();
            }
        }
        result.remote_urls = urls;
    }

    return result;
}

ConfigSettings ConfigReader::read_config() {
    auto paths = collect_config_paths();

    if (paths.empty()) {
        throw std::runtime_error("Config file not found in any of the expected locations.");
    }

    PartialConfigSettings final_partial;

    for (const auto& path : paths) {
        auto cfg = read_config_from_path(path);
        final_partial.merge(cfg);
    }

    return final_partial.into_final_settings();
}

PartialConfigSettings
ConfigReader::read_config_from_path(const std::filesystem::path& config_path) {
    std::ifstream file(config_path);
    if (!file.is_open()) {
        throw std::runtime_error(std::string("Failed to read config file: ") +
                                 config_path.string());
    }

    json j;
    try {
        file >> j;
    } catch (const json::exception& e) {
        throw std::runtime_error(std::string("Failed to deserialize config: ") + e.what());
    }

    return from_json(j);
}

std::vector<std::filesystem::path> ConfigReader::collect_config_paths() {
    // Check for GITSYNC_CONFIG_PATH environment variable
    const char* env_config = std::getenv("GITSYNC_CONFIG_PATH");
    std::optional<std::string> env_path;
    if (env_config) {
        env_path = std::string(env_config);
    }

    // Get home directory (use HOME env var on Unix)
    std::optional<std::filesystem::path> home_dir;
    const char* home_env = std::getenv("HOME");
    if (home_env) {
        home_dir = std::filesystem::path(home_env);
    }

    // Get current working directory
    std::optional<std::filesystem::path> current_dir;
    try {
        current_dir = std::filesystem::current_path();
    } catch (...) {
        // Current path might not be accessible
    }

    return resolve_config_hierarchy(env_path, home_dir, current_dir);
}

std::vector<std::filesystem::path>
ConfigReader::resolve_config_hierarchy(const std::optional<std::string>& config_env,
                                       const std::optional<std::filesystem::path>& home_dir,
                                       const std::optional<std::filesystem::path>& current_dir) {
    std::vector<std::filesystem::path> paths;

    // If GITSYNC_CONFIG_PATH is set and exists, it takes absolute priority
    if (config_env) {
        auto env_path = std::filesystem::path(*config_env);
        // Trim whitespace would require more logic; for simplicity, use as-is
        if (std::filesystem::exists(env_path)) {
            paths.push_back(env_path);
            return paths;
        }
    }

    // Global config: ~/.gitsync/config.json
    if (home_dir) {
        auto global_path = *home_dir / ".gitsync" / "config.json";
        if (std::filesystem::exists(global_path)) {
            paths.push_back(global_path);
        }
    }

    // Organization and repository configs
    if (current_dir) {
        // Organization config: ../.gitsync/config.json (parent directory)
        if (auto parent = current_dir->parent_path(); parent != current_dir) {
            auto org_path = parent / ".gitsync" / "config.json";
            if (std::filesystem::exists(org_path)) {
                // Avoid duplicates
                if (std::find(paths.begin(), paths.end(), org_path) == paths.end()) {
                    paths.push_back(org_path);
                }
            }
        }

        // Repository config: ./.gitsync/config.json (current directory)
        auto local_path = *current_dir / ".gitsync" / "config.json";
        if (std::filesystem::exists(local_path)) {
            // Avoid duplicates
            if (std::find(paths.begin(), paths.end(), local_path) == paths.end()) {
                paths.push_back(local_path);
            }
        }
    }

    return paths;
}

} // namespace aw::gitsync
