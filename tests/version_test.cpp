#include "engine/version.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("scaffold_version returns the placeholder value", "[scaffold]") {
  REQUIRE(engine::scaffold_version() == 0);
}
