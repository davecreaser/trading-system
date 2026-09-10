#include "engine/itch_framing.hpp"

#include <catch2/catch_test_macros.hpp>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

TEST_CASE("read_next_message returns one message's payload bytes", "[itch_framing]") {
  std::string bytes{static_cast<char>(0x00), static_cast<char>(0x03), 'a', 'b', 'c'};
  std::istringstream stream(bytes);

  std::optional<std::vector<unsigned char>> message = engine::read_next_message(stream);
  REQUIRE(message.has_value());
  REQUIRE(*message == std::vector<unsigned char>{'a', 'b', 'c'});
}

TEST_CASE("read_next_message returns nullopt on an exhausted stream", "[itch_framing]") {
  std::istringstream stream("");

  std::optional<std::vector<unsigned char>> message = engine::read_next_message(stream);
  REQUIRE(message == std::nullopt);
}

TEST_CASE("read_next_message throws when the stream is truncated mid-message", "[itch_framing]") {
  // Length prefix claims 5 bytes follow, but only 2 actually do.
  std::string bytes{static_cast<char>(0x00), static_cast<char>(0x05), 'a', 'b'};
  std::istringstream stream(bytes);

  REQUIRE_THROWS_AS(engine::read_next_message(stream), std::runtime_error);
}
