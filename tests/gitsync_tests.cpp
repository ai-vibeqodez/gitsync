#include <catch2/catch_test_macros.hpp>

#include "aw/gitsync/gitsync.hpp"

TEST_CASE("greet greets", "[gitsync]") {
    REQUIRE(aw::gitsync::greet("world") == "Hello, world!");
}
