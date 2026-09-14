#pragma once

#include <cstddef>
#include <optional>

#include "engine/itch_message.hpp"
#include "engine/order.hpp"
#include "engine/order_book.hpp"
#include "engine/strategy.hpp"

namespace engine {

struct NaiveMarketMakerQuote {
  std::optional<Ticks> bid;
  std::optional<Ticks> ask;
  Quantity size;
};

// Fixed-point scale for skew_coefficient_, avoiding float in the price-computation
// path (Ticks are integers for the same reason - see ADR-0005).
constexpr int64_t kSkewScaleFactor = 1000;

class NaiveMarketMaker : public Strategy {
public:
  explicit NaiveMarketMaker(Quantity quote_size, Ticks half_spread_ticks, int64_t skew_coefficient,
                            Inventory inventory_threshold);

  const NaiveMarketMakerQuote& quote() const;

  std::optional<StrategyResult> on_message(const DecodedMessage& message,
                                           const OrderBook& book) override;

private:
  Quantity quote_size_;
  Ticks half_spread_ticks_;
  // Ticks of skew per kSkewScaleFactor shares of inventory, not per single share.
  int64_t skew_coefficient_;
  Inventory inventory_threshold_;

  bool in_regular_hours_ = false;
  std::optional<Ticks> reference_price_;
  NaiveMarketMakerQuote quote_;
  Inventory current_inventory_ = 0;
};

}  // namespace engine
