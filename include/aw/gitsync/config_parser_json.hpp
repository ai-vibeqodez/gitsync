#ifndef AW_GITSYNC_CONFIG_PARSER_JSON_HPP
#define AW_GITSYNC_CONFIG_PARSER_JSON_HPP

#include <filesystem>

#include "aw/gitsync/config_settings.hpp"

namespace aw::gitsync {

/// JSON configuration parser with dual-convention support
class ConfigParserJson {
public:
    /// Parse JSON file and return PartialConfigSettings
    /// Supports both PascalCase and snake_case field names
    /// snake_case takes precedence if both present
    static PartialConfigSettings parse_file(const std::filesystem::path& path);
};

} // namespace aw::gitsync

#endif // AW_GITSYNC_CONFIG_PARSER_JSON_HPP
