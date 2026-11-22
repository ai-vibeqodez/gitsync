#include "aw/gitsync/config_settings.hpp"

namespace aw::gitsync {

void PartialConfigSettings::merge(const PartialConfigSettings& local) {
    if (local.log_level) {
        log_level = local.log_level;
    }
    if (local.remote_branch_template) {
        remote_branch_template = local.remote_branch_template;
    }
    if (local.source_remote_name) {
        source_remote_name = local.source_remote_name;
    }
    if (local.ssh_key_types) {
        ssh_key_types = local.ssh_key_types;
    }
    if (local.remote_urls) {
        if (remote_urls) {
            // Extend existing map with new entries
            for (const auto& [key, value] : *local.remote_urls) {
                (*remote_urls)[key] = value;
            }
        } else {
            remote_urls = local.remote_urls;
        }
    }
}

ConfigSettings PartialConfigSettings::into_final_settings() const {
    return ConfigSettings{
        .log_level = log_level.value_or("info"),
        .remote_branch_template =
            remote_branch_template.value_or("%owner%/%reponame%/%branchname%"),
        .remote_urls = remote_urls.value_or(std::map<std::string, std::string>{}),
        .source_remote_name = source_remote_name.value_or("origin"),
        .ssh_key_types = ssh_key_types.value_or(""),
    };
}

} // namespace aw::gitsync
