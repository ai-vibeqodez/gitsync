#include <cstdlib>
#include <format>
#include <iostream>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include <spdlog/spdlog.h>

#include "aw/gitsync/config_reader.hpp"
#include "aw/gitsync/git_sync_service.hpp"
#include "aw/gitsync/known_hosts_service.hpp"
#include "aw/gitsync/process_runner.hpp"

// Version is provided at compile time via CMake
#ifndef GITSYNC_VERSION
#    define GITSYNC_VERSION "0.0.0"
#endif

using namespace aw::gitsync;

enum class CliAction {
    Run,
    Version,
    Update,
    Help,
};

CliAction parse_args() {
    std::vector<std::string> args;
    // Note: in main we get argc/argv directly
    // This function will be refined based on actual call site
    return CliAction::Run;
}

void init_logging(const std::string& log_level) {
    // Set spdlog default level based on config
    if (log_level == "debug") {
        spdlog::set_level(spdlog::level::debug);
    } else if (log_level == "info") {
        spdlog::set_level(spdlog::level::info);
    } else if (log_level == "warn") {
        spdlog::set_level(spdlog::level::warn);
    } else if (log_level == "err" || log_level == "error") {
        spdlog::set_level(spdlog::level::err);
    } else {
        spdlog::set_level(spdlog::level::info);
    }
}

void run_app(bool skip_host) {
    auto config = ConfigReader::read_config();
    init_logging(config.log_level);

    auto process_runner = std::make_shared<ProcessRunner>();
    auto known_hosts_service = std::make_unique<KnownHostsService>(process_runner, config);
    auto git_sync_service = std::make_unique<GitSyncService>(process_runner, config);

    if (!skip_host) {
        known_hosts_service->warm_up();
    }

    git_sync_service->run();
}

void print_version() {
    std::cout << "gitsync v" << GITSYNC_VERSION << '\n';
}

void print_help() {
    std::cout << "gitsync v" << GITSYNC_VERSION << "\n\n";
    std::cout << "USAGE:\n";
    std::cout << "    gitsync [OPTIONS]\n\n";
    std::cout << "OPTIONS:\n";
    std::cout << "    -h, --help          Print help\n";
    std::cout << "    -v, --version       Print version\n";
    std::cout << "    -u, --update        Check for and apply updates\n";
    std::cout << "    -s, --skip-host     Skip SSH known_hosts warm-up\n\n";
    std::cout << "COMMANDS:\n";
    std::cout << "    gitsync             Run git sync (default)\n";
}

int main(int argc, char* argv[]) {
    try {
        std::span<char*> args(argv, static_cast<std::size_t>(argc));

        // Parse command line arguments
        bool skip_host = false;
        bool version_requested = false;
        bool help_requested = false;
        bool update_requested = false;

        for (size_t i = 1; i < args.size(); ++i) {
            std::string_view arg = args[i];
            if (arg == "-h" || arg == "--help") {
                help_requested = true;
            } else if (arg == "-v" || arg == "--version") {
                version_requested = true;
            } else if (arg == "-u" || arg == "--update") {
                update_requested = true;
            } else if (arg == "-s" || arg == "--skip-host") {
                skip_host = true;
            }
        }

        if (help_requested) {
            print_help();
            return 0;
        }

        if (version_requested) {
            print_version();
            return 0;
        }

        if (update_requested) {
            std::cerr << "Update feature not yet implemented\n";
            return 1;
        }

        // Default: run git sync
        run_app(skip_host);
        return 0;

    } catch (const std::exception& e) {
        spdlog::error("Fatal error: {}", e.what());
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
}
