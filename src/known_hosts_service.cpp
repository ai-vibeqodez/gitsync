#include "aw/gitsync/known_hosts_service.hpp"

#include <algorithm>
#include <fstream>
#include <regex>
#include <sstream>
#include <stdexcept>

#include <spdlog/spdlog.h>

namespace aw::gitsync {

KnownHostsService::KnownHostsService(std::shared_ptr<IProcessRunner> process_runner,
                                     const ConfigSettings& config)
    : process_runner_(process_runner), config_(config) {}

void KnownHostsService::warm_up() {
    auto known_hosts_path = get_known_hosts_path();
    warm_up_at(known_hosts_path);
}

void KnownHostsService::warm_up_at(const std::filesystem::path& known_hosts_path) {
    auto hosts = get_hosts();
    std::string hosts_str;
    for (size_t i = 0; i < hosts.size(); ++i) {
        if (i > 0)
            hosts_str += ", ";
        hosts_str += hosts[i];
    }
    spdlog::debug("Known hosts warmup started for: {}", hosts_str);

    auto existing = load_known_hosts(known_hosts_path);
    auto updated = build_updated_known_hosts(existing, hosts);

    save_known_hosts(known_hosts_path, updated);
    spdlog::debug("Known hosts warmup completed. Total lines: {}", updated.size());
}

std::vector<std::string> KnownHostsService::get_hosts() {
    std::vector<std::string> hosts;
    for (const auto& [name, url] : config_.remote_urls) {
        try {
            hosts.push_back(parse_host(url));
        } catch (...) {
            // Skip invalid URLs
        }
    }

    // Sort and deduplicate
    std::sort(hosts.begin(), hosts.end());
    hosts.erase(std::unique(hosts.begin(), hosts.end()), hosts.end());

    return hosts;
}

std::filesystem::path KnownHostsService::get_known_hosts_path() {
    // Try to get SSH directory
    const char* home_env = std::getenv("HOME");
    std::filesystem::path ssh_dir;
    if (home_env) {
        ssh_dir = std::filesystem::path(home_env) / ".ssh";
    } else {
        ssh_dir = std::filesystem::path(".") / ".ssh";
    }

    try {
        std::filesystem::create_directories(ssh_dir);
    } catch (...) {
        // Ignore errors in creating directory
    }

    return ssh_dir / "known_hosts";
}

std::vector<std::string>
KnownHostsService::load_known_hosts(const std::filesystem::path& known_hosts_path) {
    if (!std::filesystem::exists(known_hosts_path)) {
        return {};
    }

    std::vector<std::string> lines;
    std::ifstream file(known_hosts_path);
    std::string line;
    while (std::getline(file, line)) {
        // Trim leading/trailing whitespace
        auto start = line.begin();
        while (start != line.end() && std::isspace(*start)) {
            ++start;
        }
        auto end = line.end();
        while (end != line.begin() && std::isspace(*(end - 1))) {
            --end;
        }
        line = std::string(start, end);

        if (!line.empty()) {
            lines.push_back(line);
        }
    }
    return lines;
}

std::vector<std::string>
KnownHostsService::build_updated_known_hosts(const std::vector<std::string>& existing,
                                             const std::vector<std::string>& hosts) {
    // Keep lines that don't match any host
    std::vector<std::string> preserved;
    for (const auto& line : existing) {
        bool matches = false;
        for (const auto& host : hosts) {
            if (line_matches_host(line, host)) {
                matches = true;
                break;
            }
        }
        if (!matches) {
            preserved.push_back(line);
        }
    }

    // Add newly scanned entries
    auto scanned = scan_hosts(hosts);
    preserved.insert(preserved.end(), scanned.begin(), scanned.end());

    // Sort and deduplicate
    std::sort(preserved.begin(), preserved.end());
    preserved.erase(std::unique(preserved.begin(), preserved.end()), preserved.end());

    return preserved;
}

std::vector<std::string> KnownHostsService::scan_hosts(const std::vector<std::string>& hosts) {
    std::vector<std::string> result;

    for (const auto& host : hosts) {
        spdlog::debug("Scanning host {}", host);

        try {
            auto output = execute_scan_with_fallback(host);
            auto lines = parse_scan_output(output, host);
            result.insert(result.end(), lines.begin(), lines.end());
        } catch (const std::exception& e) {
            spdlog::warn("Failed to scan host {}: {}", host, e.what());
        }
    }

    return result;
}

ProcessOutput KnownHostsService::execute_scan_with_fallback(const std::string& host) {
    std::vector<std::string> args;

    if (!config_.ssh_key_types.empty()) {
        // Trim ssh_key_types
        std::string trimmed = config_.ssh_key_types;
        auto start = trimmed.begin();
        while (start != trimmed.end() && std::isspace(*start)) {
            ++start;
        }
        auto end = trimmed.end();
        while (end != trimmed.begin() && std::isspace(*(end - 1))) {
            --end;
        }
        trimmed = std::string(start, end);

        if (!trimmed.empty()) {
            args.push_back("-t");
            args.push_back(trimmed);
        }
    }
    args.push_back(host);

    auto output = process_runner_->run("ssh-keyscan", args);

    // If stdout is empty and we used ssh_key_types, retry without the type filter
    if (output.stdout_str.empty() && !config_.ssh_key_types.empty()) {
        spdlog::debug("Empty stdout for {} with type flag. Retrying broad scan...", host);
        try {
            auto fallback = process_runner_->run("ssh-keyscan", std::vector<std::string>{host});
            output = fallback;
        } catch (...) {
            // Use original output on fallback failure
        }
    }

    return output;
}

std::vector<std::string> KnownHostsService::parse_scan_output(const ProcessOutput& output,
                                                              const std::string& host) {
    // Normalize line endings (replace \r with nothing)
    std::string clean_stdout = output.stdout_str;
    clean_stdout.erase(std::remove(clean_stdout.begin(), clean_stdout.end(), '\r'),
                       clean_stdout.end());

    std::string clean_stderr = output.stderr_str;
    clean_stderr.erase(std::remove(clean_stderr.begin(), clean_stderr.end(), '\r'),
                       clean_stderr.end());

    std::string full_output = clean_stdout + "\n" + clean_stderr;

    std::vector<std::string> result;
    std::istringstream iss(full_output);
    std::string line;

    while (std::getline(iss, line)) {
        // Trim leading/trailing whitespace
        auto start = line.begin();
        while (start != line.end() && std::isspace(*start)) {
            ++start;
        }
        auto end = line.end();
        while (end != line.begin() && std::isspace(*(end - 1))) {
            --end;
        }
        line = std::string(start, end);

        if (!line.empty() && line_is_valid_known_host_line(line) && line_matches_host(line, host)) {
            result.push_back(line);
        }
    }

    return result;
}

void KnownHostsService::save_known_hosts(const std::filesystem::path& known_hosts_path,
                                         const std::vector<std::string>& lines) {
    std::ofstream file(known_hosts_path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to write known_hosts file");
    }

    for (const auto& line : lines) {
        file << line << "\n";
    }
}

bool KnownHostsService::line_is_valid_known_host_line(const std::string& line) {
    return !line.empty() && line[0] != '#';
}

bool KnownHostsService::line_matches_host(const std::string& line, const std::string& host) {
    // Check if line starts with host followed by space, tab, or comma
    return (line.find(host + " ") == 0) || (line.find(host + "\t") == 0) ||
           (line.find(host + ",") == 0);
}

std::string KnownHostsService::parse_host(const std::string& remote_url) {
    // Try to parse as SSH URL: git@github.com:owner/repo.git
    if (remote_url.find("git@") == 0 && remote_url.find(':') != std::string::npos) {
        auto after_at_pos = remote_url.find('@');
        if (after_at_pos != std::string::npos) {
            auto after_at = remote_url.substr(after_at_pos + 1);
            auto colon_pos = after_at.find(':');
            if (colon_pos != std::string::npos) {
                auto host = after_at.substr(0, colon_pos);
                if (!host.empty()) {
                    return host;
                }
            }
        }
    }

    // Try to parse as HTTPS/HTTP URL
    // Simple regex-free parsing: look for :// and extract host
    auto scheme_end = remote_url.find("://");
    if (scheme_end != std::string::npos) {
        auto after_scheme = remote_url.substr(scheme_end + 3);

        // Find end of host (next slash or end of string)
        auto host_end = after_scheme.find('/');
        if (host_end == std::string::npos) {
            host_end = after_scheme.length();
        }

        auto host = after_scheme.substr(0, host_end);
        if (!host.empty()) {
            return host;
        }
    }

    throw std::runtime_error("Invalid remote url: " + remote_url);
}

} // namespace aw::gitsync
