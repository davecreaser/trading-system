#include "engine/naive_market_maker.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

#include "engine/itch_message.hpp"
#include "engine/order.hpp"
#include "engine/strategy.hpp"

namespace engine {

NaiveMarketMaker::NaiveMarketMaker(Quantity quote_size, Ticks half_spread_ticks,
                                   int64_t skew_coefficient, Inventory inventory_threshold)
    : quote_size_(quote_size),
      half_spread_ticks_(half_spread_ticks),
      skew_coefficient_(skew_coefficient),
      inventory_threshold_(inventory_threshold) {}

std::optional<StrategyResult> NaiveMarketMaker::on_message(const DecodedMessage& message,
                                                           const OrderBook& book) {
  if (std::holds_alternative<SystemEvent>(message)) {
    SystemEvent system_event = std::get<SystemEvent>(message);
    if (system_event.event_code == 'Q' && !in_regular_hours_) {
      in_regular_hours_ = true;
    } else if (system_event.event_code == 'M' && in_regular_hours_) {
      in_regular_hours_ = false;
    }

    return std::nullopt;
  }

  if (!in_regular_hours_ || book.best_bid() == std::nullopt || book.best_ask() == std::nullopt) {
    return std::nullopt;
  }

  Ticks mid_price = (*book.best_ask() + *book.best_bid()) / 2;
  Ticks skew = -(skew_coefficient_ * current_inventory_) / kSkewScaleFactor;

  if (mid_price != reference_price_) {
    reference_price_ = mid_price;

    quote_.ask = *reference_price_ + half_spread_ticks_ + skew;
    quote_.bid = *reference_price_ - half_spread_ticks_ + skew;

    if (current_inventory_ >= inventory_threshold_) {
      quote_.bid = std::nullopt;
    } else if (current_inventory_ <= -inventory_threshold_) {
      quote_.ask = std::nullopt;
    }

    quote_.size = quote_size_;
  }

  return StrategyResult{std::vector<StrategyFill>{}, current_inventory_};
}

const NaiveMarketMakerQuote& NaiveMarketMaker::quote() const {
  return quote_;
};

}  // namespace engine
