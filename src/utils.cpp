#include "aw/gitsync/utils.hpp"

#include <algorithm>
#include <charconv>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace aw::gitsync {

namespace {

/// Parse a semantic version string (X.Y.Z) into a vector of integers.
std::optional<std::vector<int>> parse_version_parts(std::string_view version) {
    std::vector<int> parts;
    std::string_view remaining = version;

    // Strip leading 'v' if present
    if (!remaining.empty() && remaining[0] == 'v') {
        remaining.remove_prefix(1);
    }

    // Split by '.' and parse each part
    while (!remaining.empty()) {
        auto dot_pos = remaining.find('.');
        std::string_view part = (dot_pos == std::string_view::npos) ? remaining : remaining.substr(0, dot_pos);

        int value = 0;
        auto [ptr, ec] = std::from_chars(part.data(), part.data() + part.size(), value);

        if (ec != std::errc{}) {
            return std::nullopt;  // Parse error
        }

        parts.push_back(value);

        if (dot_pos == std::string_view::npos) {
            break;
        }

        remaining.remove_prefix(dot_pos + 1);
    }

    return parts;
}

}  // namespace

std::string parse_latest_version(std::string_view json_response, std::string_view current_version) {
    // Find "tag_name" field in JSON (with optional whitespace)
    constexpr std::string_view tag_name_key = R"("tag_name")";
    auto pos = json_response.find(tag_name_key);

    if (pos == std::string_view::npos) {
        return std::string(current_version);
    }

    // Skip past "tag_name" and find the colon and opening quote
    auto colon_pos = json_response.find(':', pos + tag_name_key.size());
    if (colon_pos == std::string_view::npos) {
        return std::string(current_version);
    }

    // Find opening quote after colon
    auto quote_pos = json_response.find('"', colon_pos);
    if (quote_pos == std::string_view::npos) {
        return std::string(current_version);
    }

    // Find closing quote
    auto end = json_response.find('"', quote_pos + 1);
    if (end == std::string_view::npos) {
        return std::string(current_version);
    }

    return std::string(json_response.substr(quote_pos + 1, end - quote_pos - 1));
}

int compare_versions(std::string_view v1, std::string_view v2) {
    auto parts1 = parse_version_parts(v1);
    auto parts2 = parse_version_parts(v2);

    if (!parts1 || !parts2) {
        return -2;  // Parse error
    }

    // Compare each part (major, minor, patch, etc.)
    size_t min_size = std::min(parts1->size(), parts2->size());

    for (size_t i = 0; i < min_size; ++i) {
        if ((*parts1)[i] > (*parts2)[i]) {
            return 1;
        }
        if ((*parts1)[i] < (*parts2)[i]) {
            return -1;
        }
    }

    // If all compared parts are equal, the longer version is greater
    if (parts1->size() > parts2->size()) {
        return 1;
    }
    if (parts1->size() < parts2->size()) {
        return -1;
    }

    return 0;  // Equal
}

}  // namespace aw::gitsync
