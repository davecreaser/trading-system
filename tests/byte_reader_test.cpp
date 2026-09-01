#include <catch2/catch_test_macros.hpp>

#include "engine/byte_reader.hpp"

TEST_CASE("read_be reads a big-endian uint16_t", "[byte_reader]") {
  std::vector<std::uint8_t> bytes{0x00, 0x03};
  std::uint16_t result = engine::read_be<std::uint16_t>(bytes, 0);
  REQUIRE(result == 3);
}

TEST_CASE("read_be reads a big-endian uint32_t", "[byte_reader]") {
  std::vector<std::uint8_t> bytes{0x00, 0x01, 0x02, 0x03};
  std::uint32_t result = engine::read_be<std::uint32_t>(bytes, 0);
  REQUIRE(result == 66051);
}

TEST_CASE("read_be reads a big-endian uint64_t", "[byte_reader]") {
  std::vector<std::uint8_t> bytes{0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
  std::uint64_t result = engine::read_be<std::uint64_t>(bytes, 0);
  REQUIRE(result == 283686952306183);
}
