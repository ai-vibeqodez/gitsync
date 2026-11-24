#include "aw/gitsync/cli_args.hpp"

#include <stdexcept>
#include <string_view>

namespace aw::gitsync {

CliArgs parse_cli_args(const std::vector<std::string>& args) {
    CliArgs result;

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
