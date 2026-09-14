#pragma once

#include <optional>
#include <vector>

#include "engine/itch_message.hpp"
#include "engine/order.hpp"
#include "engine/order_book.hpp"

namespace engine {

using Inventory = std::int64_t;

struct StrategyFill {
  Ticks price; Quantity quantity;
};

struct StrategyResult {
  std::vector<StrategyFill> fills;
  Inventory inventory;
};

class Strategy {
public:
  virtual ~Strategy() = default;
  virtual std::optional<StrategyResult> on_message(const DecodedMessage&, const OrderBook&) = 0;
};

}  // namespace engine