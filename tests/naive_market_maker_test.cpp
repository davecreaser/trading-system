#include "engine/naive_market_maker.hpp"

#include <catch2/catch_test_macros.hpp>
#include <optional>

#include "engine/itch_message.hpp"
#include "engine/order.hpp"
#include "engine/order_book.hpp"

TEST_CASE("on_message returns nullopt when the book has no valid two-sided market",
          "[naive_market_maker]") {
  engine::OrderBook book{};
  engine::NaiveMarketMaker naive_market_maker{100, 4, 2, 500};
  naive_market_maker.on_message(engine::SystemEvent{'Q'}, book);
  auto r = naive_market_maker.on_message(engine::AddOrder{14, 9001, 'B', 100, 100}, book);

  REQUIRE(r == std::nullopt);
}

TEST_CASE("on_message returns nullopt outside regular trading hours, even with a valid market",
          "[naive_market_maker]") {
  engine::OrderBook book{};
  book.add(engine::Side::Sell, 100, 100);
  book.add(engine::Side::Buy, 90, 100);

  engine::NaiveMarketMaker naive_market_maker{100, 4, 2, 500};
  auto r = naive_market_maker.on_message(engine::AddOrder{14, 9001, 'B', 100, 100}, book);

  REQUIRE(r == std::nullopt);
}

TEST_CASE("on_message produces a quote correctly centered on the mid-price",
          "[naive_market_maker]") {
  engine::OrderBook book{};
  book.add(engine::Side::Sell, 100, 100);
  book.add(engine::Side::Buy, 90, 100);

  engine::NaiveMarketMaker naive_market_maker{100, 4, 2, 500};
  naive_market_maker.on_message(engine::SystemEvent{'Q'}, book);
  auto r = naive_market_maker.on_message(engine::AddOrder{14, 9001, 'B', 100, 100}, book);

  REQUIRE(r.has_value());
  REQUIRE(naive_market_maker.quote().ask == 99);
  REQUIRE(naive_market_maker.quote().bid == 91);
  REQUIRE(naive_market_maker.quote().size == 100);
}

TEST_CASE("on_message still returns a result when the mid-price hasn't changed",
          "[naive_market_maker]") {
  engine::OrderBook book{};
  book.add(engine::Side::Sell, 100, 100);
  book.add(engine::Side::Buy, 90, 100);

  engine::NaiveMarketMaker naive_market_maker{100, 4, 2, 500};
  naive_market_maker.on_message(engine::SystemEvent{'Q'}, book);
  naive_market_maker.on_message(engine::AddOrder{14, 9001, 'B', 100, 100}, book);
  auto r = naive_market_maker.on_message(engine::AddOrder{14, 9001, 'B', 100, 100}, book);

  REQUIRE(r.has_value());
  REQUIRE(naive_market_maker.quote().ask == 99);
  REQUIRE(naive_market_maker.quote().bid == 91);
  REQUIRE(naive_market_maker.quote().size == 100);
}
