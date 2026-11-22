#ifndef AW_GITSYNC_CONFIG_READER_HPP
#define AW_GITSYNC_CONFIG_READER_HPP

#include <filesystem>
#include <vector>

#include "aw/gitsync/config_settings.hpp"

namespace aw::gitsync {

/// Reads and merges configuration from multiple sources following a three-layer hierarchy:
/// 1. ~/.gitsync/config.json (global)
/// 2. ../.gitsync/config.json (organization)
/// 3. ./.gitsync/config.json (repository)
/// An environment variable GITSYNC_CONFIG_PATH overrides all three layers.
class ConfigReader {
public:
    /// Reads configuration following the layer hierarchy and merges them.
    /// Throws if no config files are found.
    static ConfigSettings read_config();

    /// Reads a single config file and deserializes it.
    /// Throws if file read or JSON parsing fails.
    static PartialConfigSettings read_config_from_path(const std::filesystem::path& config_path);

    /// Collects configuration paths following the hierarchy.
    /// Returns paths in merge order (global → org → repo).
    static std::vector<std::filesystem::path> collect_config_paths();

    /// Resolves the config hierarchy with explicit parameters (used for testing).
    static std::vector<std::filesystem::path>
    resolve_config_hierarchy(const std::optional<std::string>& config_env,
                             const std::optional<std::filesystem::path>& home_dir,
                             const std::optional<std::filesystem::path>& current_dir);
};

} // namespace aw::gitsync

#endif // AW_GITSYNC_CONFIG_READER_HPP
