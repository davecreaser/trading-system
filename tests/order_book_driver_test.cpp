#include "engine/order_book_driver.hpp"

#include <catch2/catch_test_macros.hpp>
#include <cstdint>
#include <optional>

#include "engine/itch_message.hpp"
#include "engine/order.hpp"
#include "engine/order_book.hpp"

namespace {

constexpr std::uint16_t kAaplLocate = 14;

}  // namespace

TEST_CASE("AddOrder for the target stock adds to the book and tracks its OrderId",
          "[order_book_driver]") {
  engine::OrderBook order_book{};
  engine::OrderBookDriver order_book_driver{order_book, kAaplLocate};
  engine::AddOrder add_order{kAaplLocate, 1, 'B', 100, 100};

  std::optional<engine::AddResult> result = order_book_driver.process(add_order);

  REQUIRE(result.has_value());
  REQUIRE(order_book.resting_order(result->order_id).has_value());
}

TEST_CASE("AddOrder for a different stock is ignored", "[order_book_driver]") {
  engine::OrderBook order_book{};
  engine::OrderBookDriver order_book_driver{order_book, kAaplLocate};
  engine::AddOrder add_order{12, 9001, 'B', 100, 100};

  std::optional<engine::AddResult> result = order_book_driver.process(add_order);

  REQUIRE(result == std::nullopt);
  REQUIRE(order_book.bids().empty());
  REQUIRE(order_book.asks().empty());
}

TEST_CASE("OrderDelete for a known order cancels it and forgets the mapping",
          "[order_book_driver]") {
  engine::OrderBook order_book{};
  engine::OrderBookDriver order_book_driver{order_book, kAaplLocate};

  engine::AddOrder add_order{kAaplLocate, 9001, 'B', 100, 100};
  std::optional<engine::AddResult> add_result = order_book_driver.process(add_order);

  REQUIRE(add_result.has_value());
  REQUIRE(order_book.resting_order(add_result->order_id).has_value());

  engine::OrderDelete delete_order{9001};
  std::optional<engine::AddResult> delete_result = order_book_driver.process(delete_order);

  REQUIRE(delete_result == std::nullopt);
  REQUIRE(!order_book.resting_order(add_result->order_id).has_value());
}

TEST_CASE("OrderDelete for an unknown reference number is ignored", "[order_book_driver]") {
  engine::OrderBook order_book{};
  engine::OrderBookDriver order_book_driver{order_book, kAaplLocate};

  engine::OrderDelete delete_order{1};
  std::optional<engine::AddResult> delete_result = order_book_driver.process(delete_order);

  REQUIRE(delete_result == std::nullopt);
}

TEST_CASE("OrderCancelled reduces the resting quantity by the cancelled amount",
          "[order_book_driver]") {
  engine::OrderBook order_book{};
  engine::OrderBookDriver order_book_driver{order_book, kAaplLocate};

  engine::AddOrder add_order{kAaplLocate, 9001, 'B', 100, 100};
  std::optional<engine::AddResult> add_result = order_book_driver.process(add_order);

  REQUIRE(add_result.has_value());
  REQUIRE(order_book.resting_order(add_result->order_id).has_value());

  engine::OrderCancelled cancel_order{9001, 50};
  std::optional<engine::AddResult> cancel_result = order_book_driver.process(cancel_order);

  REQUIRE(cancel_result.has_value());
  REQUIRE(order_book.resting_order(add_result->order_id)->quantity == 50);
  REQUIRE(order_book.resting_order(add_result->order_id)->price == 100);
}

TEST_CASE("OrderCancelled that empties the order forgets its reference number",
          "[order_book_driver]") {
  engine::OrderBook order_book{};
  engine::OrderBookDriver order_book_driver{order_book, kAaplLocate};

  engine::AddOrder add_order{kAaplLocate, 9001, 'B', 100, 100};
  std::optional<engine::AddResult> add_result = order_book_driver.process(add_order);
  REQUIRE(add_result.has_value());

  // Cancel the entire resting quantity, not a partial amount.
  engine::OrderCancelled cancel_order{9001, 100};
  std::optional<engine::AddResult> cancel_result = order_book_driver.process(cancel_order);

  REQUIRE(cancel_result.has_value());
  REQUIRE(cancel_result->remaining_quantity == 0);
  REQUIRE(!order_book.resting_order(add_result->order_id).has_value());

  // A later message for the same reference number must not throw -- it should be
  // treated the same as any other unresolved reference number: a clean nullopt.
  engine::OrderDelete delete_order{9001};
  std::optional<engine::AddResult> delete_result = order_book_driver.process(delete_order);
  REQUIRE(delete_result == std::nullopt);
}

TEST_CASE("OrderCancelled for an unknown reference number is ignored", "[order_book_driver]") {
  engine::OrderBook order_book{};
  engine::OrderBookDriver order_book_driver{order_book, kAaplLocate};

  engine::OrderCancelled cancel_order{1, 50};
  std::optional<engine::AddResult> cancel_result = order_book_driver.process(cancel_order);

  REQUIRE(cancel_result == std::nullopt);
}

TEST_CASE("OrderCancelled cancelling more than the resting quantity clamps to zero",
          "[order_book_driver]") {
  engine::OrderBook order_book{};
  engine::OrderBookDriver order_book_driver{order_book, kAaplLocate};

  engine::AddOrder add_order{kAaplLocate, 9001, 'B', 100, 100};
  std::optional<engine::AddResult> add_result = order_book_driver.process(add_order);

  REQUIRE(add_result.has_value());
  REQUIRE(order_book.resting_order(add_result->order_id).has_value());

  // Requests cancelling more than the 100 shares actually resting -- a real,
  // legitimate scenario (a race between a trader's cancel request and an
  // execution that already happened), not an error.
  engine::OrderCancelled cancel_order{9001, 150};
  std::optional<engine::AddResult> cancel_result = order_book_driver.process(cancel_order);

  REQUIRE(cancel_result.has_value());
  REQUIRE(cancel_result->remaining_quantity == 0);
  REQUIRE(!order_book.resting_order(add_result->order_id).has_value());

  // The reference number should be forgotten too, same as any other full removal.
  engine::OrderDelete delete_order{9001};
  std::optional<engine::AddResult> delete_result = order_book_driver.process(delete_order);
  REQUIRE(delete_result == std::nullopt);
}

TEST_CASE("OrderReplaced updates price/quantity and re-keys the reference number",
          "[order_book_driver]") {
  engine::OrderBook order_book{};
  engine::OrderBookDriver order_book_driver{order_book, kAaplLocate};

  engine::AddOrder add_order{kAaplLocate, 9001, 'B', 100, 100};
  std::optional<engine::AddResult> add_result = order_book_driver.process(add_order);

  REQUIRE(add_result.has_value());
  REQUIRE(order_book.resting_order(add_result->order_id).has_value());

  engine::OrderReplaced replace_order{9001, 9002, 150, 200};
  std::optional<engine::AddResult> replace_result = order_book_driver.process(replace_order);

  REQUIRE(replace_result.has_value());
  REQUIRE(replace_result->remaining_quantity == 150);

  engine::OrderDelete delete_order{9002};
  std::optional<engine::AddResult> delete_result = order_book_driver.process(delete_order);

  REQUIRE(delete_result == std::nullopt);
  REQUIRE(!order_book.resting_order(add_result->order_id).has_value());
}

TEST_CASE("OrderReplaced for an unknown reference number is ignored", "[order_book_driver]") {
  engine::OrderBook order_book{};
  engine::OrderBookDriver order_book_driver{order_book, kAaplLocate};

  engine::OrderReplaced replace_order{1, 2, 150, 150};
  std::optional<engine::AddResult> replace_result = order_book_driver.process(replace_order);

  REQUIRE(replace_result == std::nullopt);
}

TEST_CASE("OrderReplaced that fully crosses does not register a stale reference number",
          "[order_book_driver]") {
  engine::OrderBook order_book{};
  engine::OrderBookDriver order_book_driver{order_book, kAaplLocate};

  // A resting Sell to cross against.
  engine::AddOrder resting_order{kAaplLocate, 9001, 'S', 100, 100};
  order_book_driver.process(resting_order);

  // A Buy that rests initially at a non-crossing price.
  engine::AddOrder order_to_replace{kAaplLocate, 9002, 'B', 50, 90};
  std::optional<engine::AddResult> add_result = order_book_driver.process(order_to_replace);
  REQUIRE(add_result.has_value());

  // Replace it with a price aggressive enough to fully cross against the resting Sell.
  engine::OrderReplaced replace_order{9002, 9003, 50, 100};
  std::optional<engine::AddResult> replace_result = order_book_driver.process(replace_order);

  REQUIRE(replace_result.has_value());
  REQUIRE(replace_result->remaining_quantity == 0);
  REQUIRE(replace_result->fills.size() == 1);

  // Nothing is resting under the new reference number -- it should not resolve.
  REQUIRE(order_book_driver.resolve(9003) == std::nullopt);

  // A later message for the new reference number must not throw -- it should be
  // treated the same as any other unresolved reference number: a clean nullopt.
  engine::OrderDelete delete_order{9003};
  std::optional<engine::AddResult> delete_result = order_book_driver.process(delete_order);
  REQUIRE(delete_result == std::nullopt);
}

TEST_CASE("resolve returns the OrderId for a tracked reference number", "[order_book_driver]") {
  engine::OrderBook order_book{};
  engine::OrderBookDriver order_book_driver{order_book, kAaplLocate};

  engine::AddOrder add_order{kAaplLocate, 9001, 'B', 100, 100};
  std::optional<engine::AddResult> add_result = order_book_driver.process(add_order);
  std::optional<engine::OrderId> id = order_book_driver.resolve(9001);

  REQUIRE(add_result.has_value());
  REQUIRE(id == add_result->order_id);
}

TEST_CASE("resolve returns nullopt for an untracked reference number", "[order_book_driver]") {
  engine::OrderBook order_book{};
  engine::OrderBookDriver order_book_driver{order_book, kAaplLocate};

  std::optional<engine::OrderId> id = order_book_driver.resolve(9001);

  REQUIRE(id == std::nullopt);
}

TEST_CASE("OrderDelete for an order already consumed by another order's crossing does not throw",
          "[order_book_driver]") {
  engine::OrderBook order_book{};
  engine::OrderBookDriver order_book_driver{order_book, kAaplLocate};

  // A resting Buy, small enough to be fully consumed by the next order.
  engine::AddOrder resting_order{kAaplLocate, 9001, 'B', 4, 100};
  std::optional<engine::AddResult> resting_result = order_book_driver.process(resting_order);
  REQUIRE(resting_result.has_value());

  // An unrelated incoming Sell that fully crosses and consumes it.
  engine::AddOrder crossing_order{kAaplLocate, 9002, 'S', 4, 100};
  std::optional<engine::AddResult> crossing_result = order_book_driver.process(crossing_order);
  REQUIRE(crossing_result.has_value());
  REQUIRE(crossing_result->fills.size() == 1);
  REQUIRE(!order_book.resting_order(resting_result->order_id).has_value());

  // A stray/late Delete for the already-consumed resting order must not throw --
  // ITCH never tells us proactively that a tracked resting order was just consumed
  // as the passive side of someone else's trade.
  engine::OrderDelete delete_order{9001};
  std::optional<engine::AddResult> delete_result = order_book_driver.process(delete_order);
  REQUIRE(delete_result == std::nullopt);
}
