#pragma once

#include <cstdint>
#include <optional>
#include <unordered_map>

#include "engine/itch_message.hpp"
#include "engine/order_book.hpp"

namespace engine {

class OrderBookDriver {
public:
  OrderBookDriver(OrderBook& book, std::uint16_t stock_locate);

  std::optional<AddResult> process(const DecodedMessage& message);

  std::optional<OrderId> resolve(std::uint64_t itch_reference_number) const;

private:
  OrderBook& book_;
  std::uint16_t stock_locate_;
  std::unordered_map<std::uint64_t, OrderId> itch_ref_to_order_id_;
};

}  // namespace engine
