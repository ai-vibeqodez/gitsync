#include "aw/gitsync/cli_args.hpp"

#include <stdexcept>
#include <string_view>
#include <thread>

namespace aw::gitsync {

CliArgs parse_cli_args(const std::vector<std::string>& args) {
    CliArgs result;
    int max_jobs = static_cast<int>(std::thread::hardware_concurrency());
    if (max_jobs <= 0) max_jobs = 32;  // Fallback if detection fails

    for (size_t i = 1; i < args.size(); ++i) {
        std::string_view arg = args[i];

        // Long-form flags
        if (arg == "--dry-run") {
            result.dry_run = true;
        } else if (arg == "--verbose" || arg == "-vv") {
            result.verbose = true;
        } else if (arg == "--quiet" || arg == "-q") {
            result.quiet = true;
        } else if (arg == "--json") {
            result.json_output = true;
        } else if (arg == "--help" || arg == "-h") {
            result.help = true;
        } else if (arg == "--version" || arg == "-v") {
            result.version = true;
        } else if (arg == "--update" || arg == "-u") {
            result.update = true;
        } else if (arg == "--skip-host" || arg == "-s") {
            result.skip_host = true;
        }
        // Jobs flag: --jobs=N or -j N
        else if (arg.starts_with("--jobs=")) {
            std::string_view value = arg.substr(7);
            try {
                result.jobs = std::stoi(std::string(value));
                if (result.jobs < 1 || result.jobs > max_jobs) {
                    throw std::runtime_error("Jobs must be between 1 and " + std::to_string(max_jobs));
                }
            } catch (const std::exception& e) {
                throw std::runtime_error(std::string("Invalid --jobs value: ") + std::string(value));
            }
        } else if (arg == "--jobs" || arg == "-j") {
            // Next argument is the value
            if (i + 1 >= args.size()) {
                throw std::runtime_error("--jobs requires a value");
            }
            ++i;
            try {
                result.jobs = std::stoi(args[i]);
                if (result.jobs < 1 || result.jobs > max_jobs) {
                    throw std::runtime_error("Jobs must be between 1 and " + std::to_string(max_jobs));
                }
            } catch (const std::exception& e) {
                throw std::runtime_error(std::string("Invalid --jobs value: ") + args[i]);
            }
        }
        // Short-form flags (-d)
        else if (arg == "-d") {
            result.dry_run = true;
        }
        // Unknown flag
        else if (arg.starts_with('-')) {
            throw std::runtime_error(std::string("Unknown flag: ") + std::string(arg));
        }
        // Positional arguments (not supported yet)
        else {
            throw std::runtime_error(std::string("Unexpected positional argument: ") +
                                     std::string(arg));
        }
    }

    // Validation: dry-run with json-output is OK, but verbose and quiet together is warning-worthy
    if (result.verbose && result.quiet) {
        throw std::runtime_error("Cannot specify both --verbose and --quiet");
    }

    return result;
}

} // namespace aw::gitsync
