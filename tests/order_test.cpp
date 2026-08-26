#include <catch2/catch_test_macros.hpp>

#include "engine/order.hpp"

TEST_CASE("Order holds the fields it was constructed with", "[order]") {
  engine::Order order{1, engine::Side::Buy, 1005, 100};
  REQUIRE(order.id == 1);
  REQUIRE(order.side == engine::Side::Buy);
  REQUIRE(order.price == 1005);
  REQUIRE(order.quantity == 100);
}

TEST_CASE("Bids sort with the highest price first", "[order]") {
  engine::Bids bids;
  bids[1000].push_back(engine::Order{1, engine::Side::Buy, 1000, 10});
  bids[1005].push_back(engine::Order{2, engine::Side::Buy, 1005, 10});
  REQUIRE(bids.begin()->first == 1005);
}

TEST_CASE("Asks sort with the lowest price first", "[order]") {
  engine::Asks asks;
  asks[1010].push_back(engine::Order{3, engine::Side::Sell, 1010, 10});
  asks[1005].push_back(engine::Order{4, engine::Side::Sell, 1005, 10});
  REQUIRE(asks.begin()->first == 1005);
}
