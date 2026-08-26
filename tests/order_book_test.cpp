#include <catch2/catch_test_macros.hpp>

#include "engine/order.hpp"
#include "engine/order_book.hpp"

TEST_CASE("Adding an order works", "[orderbook]") {
  engine::OrderBook orderbook{};

  orderbook.add(engine::Side::Buy, 1005, 100);

  const engine::Bids& bids = orderbook.bids();

  REQUIRE(bids.size() == 1);
  REQUIRE(bids.count(1005) == 1);
  REQUIRE(bids.at(1005).size() == 1);
}

TEST_CASE("Adding two orders at the same price adds them in the right order", "[orderbook]") {
  engine::OrderBook orderbook{};

  orderbook.add(engine::Side::Buy, 1005, 100);
  orderbook.add(engine::Side::Buy, 1005, 200);

  const engine::Bids& bids = orderbook.bids();

  REQUIRE(bids.size() == 1);
  REQUIRE(bids.count(1005) == 1);
  REQUIRE(bids.at(1005).size() == 2);
  REQUIRE(bids.at(1005).front().quantity == 100);
  REQUIRE(bids.at(1005).back().quantity == 200);
}

TEST_CASE("Adding orders at different prices adds them at the right ticks level", "[orderbook]") {
  engine::OrderBook orderbook{};

  orderbook.add(engine::Side::Buy, 1005, 100);
  orderbook.add(engine::Side::Buy, 1000, 100);
  orderbook.add(engine::Side::Sell, 1100, 100);

  const engine::Bids& bids = orderbook.bids();
  const engine::Asks& asks = orderbook.asks();

  REQUIRE(bids.size() == 2);
  REQUIRE(bids.count(1005) == 1);
  REQUIRE(bids.count(1000) == 1);
  REQUIRE(bids.at(1005).size() == 1);
  REQUIRE(bids.at(1000).size() == 1);

  REQUIRE(asks.size() == 1);
  REQUIRE(asks.count(1100) == 1);
  REQUIRE(asks.at(1100).size() == 1);
}

TEST_CASE("Adding orders gives a correct AddResult", "[orderbook]") {
  engine::OrderBook orderbook{};

  engine::AddResult result_A = orderbook.add(engine::Side::Sell, 1000, 100);

  REQUIRE(result_A.fills.size() == 0);
  REQUIRE(result_A.order_id == 1);
  REQUIRE(result_A.remaining_quantity == 100);

  engine::AddResult result_B = orderbook.add(engine::Side::Sell, 1000, 100);

  REQUIRE(result_B.fills.size() == 0);
  REQUIRE(result_B.order_id == 2);
  REQUIRE(result_B.remaining_quantity == 100);
}

TEST_CASE("Cancelling a non-existent order returns false and changes nothing", "[orderbook]") {
  engine::OrderBook orderbook{};

  orderbook.add(engine::Side::Buy, 1005, 100);

  bool cancelled = orderbook.cancel(999);

  REQUIRE(cancelled == false);
  REQUIRE(orderbook.bids().size() == 1);
  REQUIRE(orderbook.bids().at(1005).size() == 1);
}

TEST_CASE("Cancelling the middle order at a price level leaves the others in order", "[orderbook]") {
  engine::OrderBook orderbook{};

  engine::AddResult result_A = orderbook.add(engine::Side::Buy, 1005, 100);
  engine::AddResult result_B = orderbook.add(engine::Side::Buy, 1005, 200);
  engine::AddResult result_C = orderbook.add(engine::Side::Buy, 1005, 300);

  bool cancelled = orderbook.cancel(result_B.order_id);

  const engine::Bids& bids = orderbook.bids();

  REQUIRE(cancelled == true);
  REQUIRE(bids.at(1005).size() == 2);
  REQUIRE(bids.at(1005).front().quantity == 100);
  REQUIRE(bids.at(1005).front().id == result_A.order_id);
  REQUIRE(bids.at(1005).back().quantity == 300);
  REQUIRE(bids.at(1005).back().id == result_C.order_id);
}

TEST_CASE("Cancelling the last order at a price level removes the level entirely", "[orderbook]") {
  engine::OrderBook orderbook{};

  engine::AddResult result = orderbook.add(engine::Side::Buy, 1005, 100);

  bool cancelled = orderbook.cancel(result.order_id);

  REQUIRE(cancelled == true);
  REQUIRE(orderbook.bids().empty());
  REQUIRE(orderbook.bids().count(1005) == 0);
}

TEST_CASE("best_bid and best_ask are empty on an empty book", "[orderbook]") {
  engine::OrderBook orderbook{};

  REQUIRE(orderbook.best_bid() == std::nullopt);
  REQUIRE(orderbook.best_ask() == std::nullopt);
}

TEST_CASE("best_bid has a value and best_ask is empty on a one-sided book", "[orderbook]") {
  engine::OrderBook orderbook{};

  orderbook.add(engine::Side::Buy, 1005, 100);

  REQUIRE(orderbook.best_bid() == 1005);
  REQUIRE(orderbook.best_ask() == std::nullopt);
}

TEST_CASE("best_bid updates when a better-priced order is added", "[orderbook]") {
  engine::OrderBook orderbook{};

  orderbook.add(engine::Side::Buy, 1000, 100);
  REQUIRE(orderbook.best_bid() == 1000);

  orderbook.add(engine::Side::Buy, 1005, 100);
  REQUIRE(orderbook.best_bid() == 1005);
}

TEST_CASE("best_bid falls back to the next price level when the top level is cancelled", "[orderbook]") {
  engine::OrderBook orderbook{};

  orderbook.add(engine::Side::Buy, 1000, 100);
  engine::AddResult top = orderbook.add(engine::Side::Buy, 1005, 100);

  REQUIRE(orderbook.best_bid() == 1005);

  orderbook.cancel(top.order_id);

  REQUIRE(orderbook.best_bid() == 1000);
}

TEST_CASE("Crossing: exact-quantity match fully consumes both orders", "[orderbook]") {
  engine::OrderBook orderbook{};

  engine::Ticks price = 1000;
  engine::Quantity quantity = 100;

  engine::AddResult resting = orderbook.add(engine::Side::Sell, price, quantity);
  engine::AddResult incoming = orderbook.add(engine::Side::Buy, price, quantity);

  REQUIRE(incoming.fills.size() == 1);
  std::vector<engine::Fill>::iterator fill = incoming.fills.begin();
  REQUIRE(fill->incoming_order_id == incoming.order_id);
  REQUIRE(fill->resting_order_id == resting.order_id);
  REQUIRE(fill->price == price);
  REQUIRE(fill->quantity == quantity);
  REQUIRE(incoming.remaining_quantity == 0);
  REQUIRE(orderbook.asks().size() == 0);
  REQUIRE(orderbook.bids().size() == 0);
}

TEST_CASE("Crossing: incoming larger than resting rests the leftover", "[orderbook]") {
  engine::OrderBook orderbook{};

  engine::Ticks price = 1000;
  engine::Quantity quantity_resting = 100;
  engine::Quantity quantity_incoming = 150;

  engine::AddResult resting = orderbook.add(engine::Side::Sell, price, quantity_resting);
  engine::AddResult incoming = orderbook.add(engine::Side::Buy, price, quantity_incoming);

  REQUIRE(incoming.fills.size() == 1);
  std::vector<engine::Fill>::iterator fill = incoming.fills.begin();
  REQUIRE(fill->incoming_order_id == incoming.order_id);
  REQUIRE(fill->resting_order_id == resting.order_id);
  REQUIRE(fill->price == price);
  REQUIRE(fill->quantity == quantity_resting);

  REQUIRE(incoming.remaining_quantity == quantity_incoming - quantity_resting);
  
  REQUIRE(orderbook.asks().size() == 0);

  const engine::Bids& bids = orderbook.bids();
  REQUIRE(bids.size() == 1);
  REQUIRE(bids.count(price) == 1);
  REQUIRE(bids.at(price).size() == 1);
  REQUIRE(bids.at(price).front().quantity == incoming.remaining_quantity);
  REQUIRE(bids.at(price).front().id == incoming.order_id);
}

TEST_CASE("Crossing: incoming smaller than resting leaves it resting at reduced quantity", "[orderbook]") {
  engine::OrderBook orderbook{};

  engine::Ticks price = 1000;
  engine::Quantity quantity_resting = 100;
  engine::Quantity quantity_incoming = 40;

  engine::AddResult resting = orderbook.add(engine::Side::Sell, price, quantity_resting);
  engine::AddResult incoming = orderbook.add(engine::Side::Buy, price, quantity_incoming);

  REQUIRE(incoming.fills.size() == 1);
  std::vector<engine::Fill>::iterator fill = incoming.fills.begin();
  REQUIRE(fill->incoming_order_id == incoming.order_id);
  REQUIRE(fill->resting_order_id == resting.order_id);
  REQUIRE(fill->price == price);
  REQUIRE(fill->quantity == quantity_incoming);

  REQUIRE(incoming.remaining_quantity == 0);

  const engine::Asks& asks = orderbook.asks();
  REQUIRE(asks.size() == 1);
  REQUIRE(asks.count(price) == 1);
  REQUIRE(asks.at(price).size() == 1);
  REQUIRE(asks.at(price).front().quantity == quantity_resting - quantity_incoming);
  REQUIRE(asks.at(price).front().id == resting.order_id);

  REQUIRE(orderbook.bids().size() == 0);
}

TEST_CASE("Crossing: a Sell crossing into bids works the same way", "[orderbook]") {
  engine::OrderBook orderbook{};

  engine::Ticks price = 1005;
  engine::Quantity quantity = 100;

  engine::AddResult resting = orderbook.add(engine::Side::Buy, price, quantity);
  engine::AddResult incoming = orderbook.add(engine::Side::Sell, price, quantity);

  REQUIRE(incoming.fills.size() == 1);
  std::vector<engine::Fill>::iterator fill = incoming.fills.begin();
  REQUIRE(fill->incoming_order_id == incoming.order_id);
  REQUIRE(fill->resting_order_id == resting.order_id);
  REQUIRE(fill->price == price);
  REQUIRE(fill->quantity == quantity);

  REQUIRE(incoming.remaining_quantity == 0);

  REQUIRE(orderbook.bids().size() == 0);
  REQUIRE(orderbook.asks().size() == 0);
}
