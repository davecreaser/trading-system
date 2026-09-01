#include <catch2/catch_test_macros.hpp>

#include "engine/itch_message.hpp"

TEST_CASE("decode_stock_directory decodes a real Stock Directory message", "[itch_message]") {
  // the second message in the real ITCH sample file (data/itch_sample_slice.bin):
  // Message Type 'R', Stock Locate 1, symbol "A" (Agilent Technologies, NYSE)
  std::vector<std::uint8_t> bytes{
      0x52, 0x00, 0x01, 0x00, 0x00, 0x0a, 0x4a, 0x4c, 0xee, 0x55,
      0x99, 0x41, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x4e,
      0x20, 0x00, 0x00, 0x00, 0x64, 0x4e, 0x43, 0x5a, 0x20, 0x50,
      0x4e, 0x20, 0x31, 0x4e, 0x00, 0x00, 0x00, 0x00, 0x4e};

  engine::StockDirectory result = engine::decode_stock_directory(bytes);

  REQUIRE(result.stock_locate == 1);
  REQUIRE(result.stock == "A");
}

TEST_CASE("decode_order_delete decodes a hand-crafted Order Delete message", "[itch_message]") {
  // Order Delete layout (engine/docs/itch-5.0-message-formats.md):
  // offset 0: Message Type "D", offset 1-2: Stock Locate, offset 3-4: Tracking Number,
  // offset 5-10: Timestamp, offset 11-18: Order Reference Number (8 bytes)
  std::vector<std::uint8_t> bytes{
      0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};

  engine::OrderDelete result = engine::decode_order_delete(bytes);

  REQUIRE(result.order_reference_number == 72623859790382856);
}

TEST_CASE("decode_message dispatches a Stock Directory message correctly", "[itch_message]") {
  // same real Stock Directory bytes as above
  std::vector<std::uint8_t> bytes{
      0x52, 0x00, 0x01, 0x00, 0x00, 0x0a, 0x4a, 0x4c, 0xee, 0x55,
      0x99, 0x41, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x4e,
      0x20, 0x00, 0x00, 0x00, 0x64, 0x4e, 0x43, 0x5a, 0x20, 0x50,
      0x4e, 0x20, 0x31, 0x4e, 0x00, 0x00, 0x00, 0x00, 0x4e};

  engine::DecodedMessage result = engine::decode_message(bytes);

  REQUIRE(std::holds_alternative<engine::StockDirectory>(result));
}

TEST_CASE("decode_message dispatches an Order Delete message correctly", "[itch_message]") {
  // same hand-crafted Order Delete bytes as above
  std::vector<std::uint8_t> bytes{
      0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};

  engine::DecodedMessage result = engine::decode_message(bytes);

  REQUIRE(std::holds_alternative<engine::OrderDelete>(result));
}

TEST_CASE("decode_message falls through to UnknownMessage for an unrecognized type", "[itch_message]") {
  std::vector<std::uint8_t> bytes{0x5a, 0x01, 0x02, 0x03};

  engine::DecodedMessage result = engine::decode_message(bytes);

  REQUIRE(std::holds_alternative<engine::UnknownMessage>(result));
  REQUIRE(std::get<engine::UnknownMessage>(result).message_type == 0x5a);
  REQUIRE(std::get<engine::UnknownMessage>(result).bytes == bytes);
}
