#include "aw/gitsync/config_reader.hpp"

#include <cstdlib>
#include <filesystem>
#include <optional>
#include <stdexcept>

#include "aw/gitsync/config_parser_json.hpp"
#include "aw/gitsync/config_parser_yaml.hpp"

namespace aw::gitsync {

namespace {
// Platform-safe getenv wrapper
std::optional<std::string> safe_getenv(const char* name) {
#ifdef _WIN32
    // Use secure _dupenv_s on Windows
    char* buffer = nullptr;
    size_t size = 0;
    if (_dupenv_s(&buffer, &size, name) == 0 && buffer != nullptr) {
        std::string result(buffer);
        free(buffer);
        return result;
    }
    return std::nullopt;
#else
    const char* value = std::getenv(name);
    if (value != nullptr) {
        return std::string(value);
    }
    return std::nullopt;
#endif
}
} // namespace

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

PartialConfigSettings ConfigReader::read_config_from_path(const std::filesystem::path& config_path) {
    // Dispatch to appropriate parser based on file extension
    auto ext = config_path.extension();
    
    if (ext == ".yaml" || ext == ".yml") {
        return ConfigParserYaml::parse_file(config_path);
    } else {
        // Default to JSON
        return ConfigParserJson::parse_file(config_path);
    }
}

std::vector<std::filesystem::path> ConfigReader::collect_config_paths() {
    // Check for GITSYNC_CONFIG_PATH environment variable
    auto env_path = safe_getenv("GITSYNC_CONFIG_PATH");

    // Get home directory (use HOME env var on Unix, USERPROFILE on Windows)
    std::optional<std::filesystem::path> home_dir;
#ifdef _WIN32
    if (auto userprofile = safe_getenv("USERPROFILE")) {
        home_dir = std::filesystem::path(*userprofile);
    }
#else
    if (auto home = safe_getenv("HOME")) {
        home_dir = std::filesystem::path(*home);
    }
#endif

    // Get current working directory
    std::optional<std::filesystem::path> current_dir;
    try {
        current_dir = std::filesystem::current_path();
    } catch (...) {
        // Current path might not be accessible
    }

    return resolve_config_hierarchy(env_path, home_dir, current_dir);
}

std::vector<std::filesystem::path> ConfigReader::resolve_config_hierarchy(
    const std::optional<std::string>& config_env,
    const std::optional<std::filesystem::path>& home_dir,
    const std::optional<std::filesystem::path>& current_dir) {
    std::vector<std::filesystem::path> paths;

    // If GITSYNC_CONFIG_PATH is set and exists, it takes absolute priority
    if (config_env) {
        auto env_path = std::filesystem::path(*config_env);
        if (std::filesystem::exists(env_path)) {
            paths.push_back(env_path);
            return paths;
        }
    }

    // Global config: ~/.gitsync/config.json or config.yaml
    if (home_dir) {
        auto json_path = *home_dir / ".gitsync" / "config.json";
        auto yaml_path = *home_dir / ".gitsync" / "config.yaml";
        auto yml_path = *home_dir / ".gitsync" / "config.yml";

        if (std::filesystem::exists(json_path)) {
            paths.push_back(json_path);
        } else if (std::filesystem::exists(yaml_path)) {
            paths.push_back(yaml_path);
        } else if (std::filesystem::exists(yml_path)) {
            paths.push_back(yml_path);
        }
    }

    // Organization and repository configs
    if (current_dir) {
        // Organization config: ../.gitsync/config.json or config.yaml (parent directory)
        if (auto parent = current_dir->parent_path(); parent != current_dir) {
            auto json_path = parent / ".gitsync" / "config.json";
            auto yaml_path = parent / ".gitsync" / "config.yaml";
            auto yml_path = parent / ".gitsync" / "config.yml";

            if (std::filesystem::exists(json_path)) {
                if (std::find(paths.begin(), paths.end(), json_path) == paths.end()) {
                    paths.push_back(json_path);
                }
            } else if (std::filesystem::exists(yaml_path)) {
                if (std::find(paths.begin(), paths.end(), yaml_path) == paths.end()) {
                    paths.push_back(yaml_path);
                }
            } else if (std::filesystem::exists(yml_path)) {
                if (std::find(paths.begin(), paths.end(), yml_path) == paths.end()) {
                    paths.push_back(yml_path);
                }
            }
        }

        // Repository config: ./.gitsync/config.json or config.yaml (current directory)
        auto json_path = *current_dir / ".gitsync" / "config.json";
        auto yaml_path = *current_dir / ".gitsync" / "config.yaml";
        auto yml_path = *current_dir / ".gitsync" / "config.yml";

        if (std::filesystem::exists(json_path)) {
            if (std::find(paths.begin(), paths.end(), json_path) == paths.end()) {
                paths.push_back(json_path);
            }
        } else if (std::filesystem::exists(yaml_path)) {
            if (std::find(paths.begin(), paths.end(), yaml_path) == paths.end()) {
                paths.push_back(yaml_path);
            }
        } else if (std::filesystem::exists(yml_path)) {
            if (std::find(paths.begin(), paths.end(), yml_path) == paths.end()) {
                paths.push_back(yml_path);
            }
        }
    }

    return paths;
}

} // namespace aw::gitsync
