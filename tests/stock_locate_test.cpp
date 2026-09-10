#include "engine/stock_locate.hpp"

#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace {

// Prepends the 2-byte big-endian length prefix read_next_message expects, so a
// TEST_CASE can build a stream out of several framed messages back to back.
std::string frame(std::vector<std::uint8_t> payload) {
  std::string framed;
  framed.push_back(static_cast<char>((payload.size() >> 8) & 0xff));
  framed.push_back(static_cast<char>(payload.size() & 0xff));
  framed.insert(framed.end(), payload.begin(), payload.end());
  return framed;
}

// Builds a Stock Directory payload with only the two fields decode_stock_directory
// actually reads (stock_locate at offset 1-2, stock at offset 11-18) populated;
// every other byte is zero, since nothing downstream looks at them.
std::string stock_directory_message(std::uint16_t stock_locate, const std::string& symbol) {
  std::vector<std::uint8_t> bytes(19, 0x00);
  bytes[0] = 'R';
  bytes[1] = static_cast<std::uint8_t>((stock_locate >> 8) & 0xff);
  bytes[2] = static_cast<std::uint8_t>(stock_locate & 0xff);

  std::string padded_symbol = symbol;
  padded_symbol.resize(8, ' ');
  for (std::size_t i = 0; i < 8; ++i) {
    bytes[11 + i] = static_cast<std::uint8_t>(padded_symbol[i]);
  }

  return frame(bytes);
}

// A minimal non-Stock-Directory message: 'S' (System Event) falls through
// decode_message's default case, which only captures raw bytes with no further
// field indexing, so a single-byte payload is safe filler.
std::string non_directory_message() {
  return frame({'S'});
}

}  // namespace

TEST_CASE("resolve_stock_locate finds the matching symbol's stock_locate", "[stock_locate]") {
  std::string bytes = non_directory_message() + stock_directory_message(1, "A") +
                      stock_directory_message(2, "AAL") + stock_directory_message(14, "AAPL");
  std::istringstream stream(bytes);

  std::optional<std::uint16_t> result = engine::resolve_stock_locate(stream, "AAPL");

  REQUIRE(result.has_value());
  REQUIRE(*result == 14);
}

TEST_CASE("resolve_stock_locate returns nullopt once the directory section ends without a match",
          "[stock_locate]") {
  std::string bytes = non_directory_message() + stock_directory_message(1, "A") +
                      stock_directory_message(2, "AAL") + non_directory_message();
  std::istringstream stream(bytes);

  std::optional<std::uint16_t> result = engine::resolve_stock_locate(stream, "AAPL");

  REQUIRE(result == std::nullopt);
}

TEST_CASE("resolve_stock_locate returns nullopt if the stream ends mid-directory-section",
          "[stock_locate]") {
  std::string bytes =
      non_directory_message() + stock_directory_message(1, "A") + stock_directory_message(2, "AAL");
  std::istringstream stream(bytes);

  std::optional<std::uint16_t> result = engine::resolve_stock_locate(stream, "AAPL");

  REQUIRE(result == std::nullopt);
}
