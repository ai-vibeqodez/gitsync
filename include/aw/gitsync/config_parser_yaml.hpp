#ifndef AW_GITSYNC_CONFIG_PARSER_YAML_HPP
#define AW_GITSYNC_CONFIG_PARSER_YAML_HPP

#include <filesystem>

#include "aw/gitsync/config_settings.hpp"

namespace aw::gitsync {

/// YAML configuration parser with dual-convention support
class ConfigParserYaml {
public:
    /// Parse YAML file and return PartialConfigSettings
    /// Supports both PascalCase and snake_case field names
    /// snake_case takes precedence if both present
    static PartialConfigSettings parse_file(const std::filesystem::path& path);
};

} // namespace aw::gitsync

#endif // AW_GITSYNC_CONFIG_PARSER_YAML_HPP
