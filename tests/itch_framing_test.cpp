#include <catch2/catch_test_macros.hpp>

#include "engine/itch_framing.hpp"

#include <sstream>
#include <optional>

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
