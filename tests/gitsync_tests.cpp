// Simple unit tests for gitsync without Catch2 framework
// Can be compiled and run without external test framework

#include "aw/gitsync/config_settings.hpp"
#include "aw/gitsync/git_sync_service.hpp"
#include "aw/gitsync/known_hosts_service.hpp"

#include <cassert>
#include <iostream>
#include <map>
#include <string>

using namespace aw::gitsync;

void test_partial_config_merge() {
    std::cout << "Test: PartialConfigSettings::merge..." << std::flush;
    PartialConfigSettings base;
    base.log_level = "info";
    base.source_remote_name = "origin";

    PartialConfigSettings local;
    local.log_level = "debug";
    local.remote_urls = std::map<std::string, std::string>{{"m1", "url1"}};

    base.merge(local);

    assert(base.log_level == "debug");
    assert(base.source_remote_name == "origin");
    assert(base.remote_urls.has_value());
    std::cout << " PASS\n";
}

void test_into_final_settings_defaults() {
    std::cout << "Test: into_final_settings with defaults..." << std::flush;
    PartialConfigSettings partial;
    auto final = partial.into_final_settings();

    assert(final.log_level == "info");
    assert(final.source_remote_name == "origin");
    assert(final.remote_branch_template == "%owner%/%reponame%/%branchname%");
    assert(final.ssh_key_types.empty());
    assert(final.remote_urls.empty());
    std::cout << " PASS\n";
}

void test_sanitize_branch_name() {
    std::cout << "Test: sanitize_branch_name..." << std::flush;
    assert(GitSyncService::sanitize_branch_name("feature/test") == "feature/test");
    assert(GitSyncService::sanitize_branch_name(".github/main") == "_github/main");
    assert(GitSyncService::sanitize_branch_name("feat@2.0!") == "feat_2_0_");
    assert(GitSyncService::sanitize_branch_name("foo..bar/baz") == "foo__bar/baz");
    std::cout << " PASS\n";
}

void test_build_remote_branch_ssh() {
    std::cout << "Test: build_remote_branch with SSH URL..." << std::flush;
    auto result = GitSyncService::build_remote_branch(
        "%owner%/%reponame%/%branchname%",
        "git@github.com:alice/project.git",
        "main");
    assert(result == "alice/project/main");
    std::cout << " PASS\n";
}

void test_build_remote_branch_https() {
    std::cout << "Test: build_remote_branch with HTTPS URL..." << std::flush;
    auto result = GitSyncService::build_remote_branch(
        "%owner%/%reponame%/%branchname%",
        "https://github.com/alice/project.git",
        "main");
    assert(result == "alice/project/main");
    std::cout << " PASS\n";
}

void test_parse_host_ssh() {
    std::cout << "Test: parse_host from SSH URL..." << std::flush;
    auto host = KnownHostsService::parse_host("git@github.com:owner/repo.git");
    assert(host == "github.com");
    std::cout << " PASS\n";
}

void test_parse_host_https() {
    std::cout << "Test: parse_host from HTTPS URL..." << std::flush;
    auto host = KnownHostsService::parse_host("https://github.com/owner/repo.git");
    assert(host == "github.com");
    std::cout << " PASS\n";
}

void test_line_matches_host() {
    std::cout << "Test: line_matches_host..." << std::flush;
    assert(KnownHostsService::line_matches_host("github.com ssh-ed25519 AAAA", "github.com"));
    assert(KnownHostsService::line_matches_host("github.com,1.2.3.4 ssh-ed25519 AAAA",
                                                 "github.com"));
    assert(!KnownHostsService::line_matches_host("gitlab.com ssh-ed25519 AAAA", "github.com"));
    std::cout << " PASS\n";
}

void test_line_is_valid_known_host_line() {
    std::cout << "Test: line_is_valid_known_host_line..." << std::flush;
    assert(KnownHostsService::line_is_valid_known_host_line("github.com ssh-ed25519 AAAA"));
    assert(!KnownHostsService::line_is_valid_known_host_line("# comment"));
    assert(!KnownHostsService::line_is_valid_known_host_line("   "));
    std::cout << " PASS\n";
}

int main() {
    std::cout << "\n=== Running gitsync unit tests ===\n\n";

    try {
        test_partial_config_merge();
        test_into_final_settings_defaults();
        test_sanitize_branch_name();
        test_build_remote_branch_ssh();
        test_build_remote_branch_https();
        test_parse_host_ssh();
        test_parse_host_https();
        test_line_matches_host();
        test_line_is_valid_known_host_line();

        std::cout << "\n=== All tests passed! ===\n\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\nTest failed with exception: " << e.what() << "\n";
        return 1;
    }
}
