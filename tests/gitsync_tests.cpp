#include "aw/gitsync/config_settings.hpp"
#include "aw/gitsync/git_sync_service.hpp"
#include "aw/gitsync/known_hosts_service.hpp"

#include <catch2/catch_test_macros.hpp>
#include <map>
#include <string>

using namespace aw::gitsync;

TEST_CASE("PartialConfigSettings::merge", "[config]") {
    PartialConfigSettings base;
    base.log_level = "info";
    base.source_remote_name = "origin";

    PartialConfigSettings local;
    local.log_level = "debug";
    local.remote_urls = std::map<std::string, std::string>{{"m1", "url1"}};

    base.merge(local);

    REQUIRE(base.log_level == "debug");
    REQUIRE(base.source_remote_name == "origin");
    REQUIRE(base.remote_urls.has_value());
}

TEST_CASE("into_final_settings with defaults", "[config]") {
    PartialConfigSettings partial;
    auto final = partial.into_final_settings();

    REQUIRE(final.log_level == "info");
    REQUIRE(final.source_remote_name == "origin");
    REQUIRE(final.remote_branch_template == "%owner%/%reponame%/%branchname%");
    REQUIRE(final.ssh_key_types.empty());
    REQUIRE(final.remote_urls.empty());
}

TEST_CASE("sanitize_branch_name", "[git]") {
    REQUIRE(GitSyncService::sanitize_branch_name("feature/test") == "feature/test");
    REQUIRE(GitSyncService::sanitize_branch_name(".github/main") == "_github/main");
    REQUIRE(GitSyncService::sanitize_branch_name("feat@2.0!") == "feat_2_0_");
    REQUIRE(GitSyncService::sanitize_branch_name("foo..bar/baz") == "foo__bar/baz");
}

TEST_CASE("build_remote_branch with SSH URL", "[git]") {
    auto result = GitSyncService::build_remote_branch(
        "%owner%/%reponame%/%branchname%",
        "git@github.com:alice/project.git",
        "main");
    REQUIRE(result == "alice/project/main");
}

TEST_CASE("build_remote_branch with HTTPS URL", "[git]") {
    auto result = GitSyncService::build_remote_branch(
        "%owner%/%reponame%/%branchname%",
        "https://github.com/alice/project.git",
        "main");
    REQUIRE(result == "alice/project/main");
}

TEST_CASE("parse_host from SSH URL", "[known_hosts]") {
    auto host = KnownHostsService::parse_host("git@github.com:owner/repo.git");
    REQUIRE(host == "github.com");
}

TEST_CASE("parse_host from HTTPS URL", "[known_hosts]") {
    auto host = KnownHostsService::parse_host("https://github.com/owner/repo.git");
    REQUIRE(host == "github.com");
}

TEST_CASE("line_matches_host", "[known_hosts]") {
    REQUIRE(KnownHostsService::line_matches_host("github.com ssh-ed25519 AAAA", "github.com"));
    REQUIRE(KnownHostsService::line_matches_host("github.com,1.2.3.4 ssh-ed25519 AAAA",
                                                  "github.com"));
    REQUIRE_FALSE(KnownHostsService::line_matches_host("gitlab.com ssh-ed25519 AAAA", "github.com"));
}

TEST_CASE("line_is_valid_known_host_line", "[known_hosts]") {
    REQUIRE(KnownHostsService::line_is_valid_known_host_line("github.com ssh-ed25519 AAAA"));
    REQUIRE_FALSE(KnownHostsService::line_is_valid_known_host_line("# comment"));
    REQUIRE_FALSE(KnownHostsService::line_is_valid_known_host_line("   "));
}
