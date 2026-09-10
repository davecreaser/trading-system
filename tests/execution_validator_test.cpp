#include "engine/execution_validator.hpp"

#include <catch2/catch_test_macros.hpp>
#include <cstdint>
#include <optional>

#include "engine/itch_message.hpp"
#include "engine/order.hpp"
#include "engine/order_book.hpp"
#include "engine/order_book_driver.hpp"

namespace {

constexpr std::uint16_t kAaplLocate = 14;

}  // namespace

TEST_CASE("record accumulates matching book and ITCH totals with no mismatch",
          "[execution_validator]") {
  engine::OrderBook order_book{};
  engine::OrderBookDriver order_book_driver{order_book, kAaplLocate};
  engine::ExecutionValidator validator{order_book_driver};

  engine::AddOrder message_1 = engine::AddOrder{kAaplLocate, 9001, 'B', 100, 100};

  std::optional<engine::AddResult> result_1 = order_book_driver.process(message_1);
  validator.record(message_1, result_1);

  engine::AddOrder message_2 = engine::AddOrder{kAaplLocate, 9002, 'S', 100, 100};

  std::optional<engine::AddResult> result_2 = order_book_driver.process(message_2);
  validator.record(message_2, result_2);

  engine::OrderExecuted message_3 = engine::OrderExecuted{9001, 100};

  std::optional<engine::AddResult> result_3 = order_book_driver.process(message_3);
  validator.record(message_3, result_3);

  REQUIRE(validator.summary().mismatches.empty());
  REQUIRE(validator.summary().orders_checked == 1);
}

TEST_CASE("record surfaces a mismatch when totals disagree", "[execution_validator]") {
  // TODO: same setup, but make the OrderExecuted message report a different
  // quantity than the Fill actually produced. REQUIRE summary().mismatches has
  // exactly one entry, with the order_id, book_total, and itch_total you expect.
  engine::OrderBook order_book{};
  engine::OrderBookDriver order_book_driver{order_book, kAaplLocate};
  engine::ExecutionValidator validator{order_book_driver};

  engine::AddOrder message_1 = engine::AddOrder{kAaplLocate, 9001, 'B', 100, 100};

  std::optional<engine::AddResult> result_1 = order_book_driver.process(message_1);
  validator.record(message_1, result_1);

  engine::AddOrder message_2 = engine::AddOrder{kAaplLocate, 9002, 'S', 100, 100};

  std::optional<engine::AddResult> result_2 = order_book_driver.process(message_2);
  validator.record(message_2, result_2);

  engine::OrderExecuted message_3 = engine::OrderExecuted{9001, 70};

  std::optional<engine::AddResult> result_3 = order_book_driver.process(message_3);
  validator.record(message_3, result_3);

  REQUIRE(validator.summary().mismatches.size() == 1);
  REQUIRE(validator.summary().mismatches.begin()->order_id == result_1->order_id);
  REQUIRE(validator.summary().mismatches.begin()->book_total == 100);
  REQUIRE(validator.summary().mismatches.begin()->itch_total == 70);
  REQUIRE(validator.summary().orders_checked == 1);
}

TEST_CASE("record ignores execution messages for an unresolved reference number",
          "[execution_validator]") {
  engine::OrderBook order_book{};
  engine::OrderBookDriver order_book_driver{order_book, kAaplLocate};
  engine::ExecutionValidator validator{order_book_driver};

  engine::OrderExecuted message = engine::OrderExecuted{9999, 100};
  std::optional<engine::AddResult> result = order_book_driver.process(message);
  validator.record(message, result);

  REQUIRE(validator.summary().mismatches.empty());
  REQUIRE(validator.summary().orders_checked == 0);
}
