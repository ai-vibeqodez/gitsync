#include "aw/gitsync/filter_matcher.hpp"

namespace aw::gitsync {

bool FilterMatcher::glob_match(std::string_view pattern, std::string_view name) {
    size_t p = 0, n = 0;

    while (p < pattern.size() && n < name.size()) {
        if (pattern[p] == '*') {
            // * matches zero or more characters
            if (p + 1 >= pattern.size()) {
                // Trailing *, matches rest of string
                return true;
            }
            // Try to match the rest of pattern at each position in name
            for (size_t i = n; i <= name.size(); ++i) {
                if (glob_match(pattern.substr(p + 1), name.substr(i))) {
                    return true;
                }
            }
            return false;
        } else if (pattern[p] == '?') {
            // ? matches exactly one character
            ++p;
            ++n;
        } else if (pattern[p] == name[n]) {
            // Exact character match
            ++p;
            ++n;
        } else {
            // No match
            return false;
        }
    }

    // Handle remaining * at end of pattern
    while (p < pattern.size() && pattern[p] == '*') {
        ++p;
    }

    // Both must be exhausted for a match
    return p == pattern.size() && n == name.size();
}

bool FilterMatcher::matches_include(const std::string& name,
                                   const std::vector<std::string>& include_patterns) {
    if (include_patterns.empty()) {
        // Empty include list = include everything
        return true;
    }

    for (const auto& pattern : include_patterns) {
        if (glob_match(pattern, name)) {
            return true;
        }
    }
    return false;
}

bool FilterMatcher::matches_exclude(const std::string& name,
                                   const std::vector<std::string>& exclude_patterns) {
    for (const auto& pattern : exclude_patterns) {
        if (glob_match(pattern, name)) {
            return true;
        }
    }
    return false;
}

bool FilterMatcher::passes_filters(const std::string& name,
                                  const std::vector<std::string>& include_patterns,
                                  const std::vector<std::string>& exclude_patterns) {
    return matches_include(name, include_patterns) && !matches_exclude(name, exclude_patterns);
}

} // namespace aw::gitsync
