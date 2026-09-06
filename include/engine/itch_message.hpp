#pragma once

#include <cstdint>
#include <span>
#include <string>
#include <variant>
#include <vector>

#include "engine/order.hpp"

namespace engine {

struct UnknownMessage {
  std::uint8_t message_type;
  std::vector<std::uint8_t> bytes;
};

struct StockDirectory {
  std::uint16_t stock_locate;
  std::string stock;
};

StockDirectory decode_stock_directory(std::span<const std::uint8_t> bytes);

struct OrderDelete {
  std::uint64_t order_reference_number;
};

OrderDelete decode_order_delete(std::span<const std::uint8_t> bytes);

struct AddOrder {
  std::uint16_t stock_locate;
  std::uint64_t order_reference_number;
  char side;
  Quantity quantity;
  Ticks price;
};

AddOrder decode_add_order(std::span<const std::uint8_t> bytes);

struct OrderExecuted {
  std::uint64_t order_reference_number;
  Quantity quantity;
};

OrderExecuted decode_order_executed(std::span<const std::uint8_t> bytes);

struct OrderExecutedWithPrice {
  std::uint64_t order_reference_number;
  Quantity quantity;
  Ticks price;
};

OrderExecutedWithPrice decode_order_executed_with_price(std::span<const std::uint8_t> bytes);

struct OrderCancelled {
  std::uint64_t order_reference_number;
  Quantity quantity;
};

OrderCancelled decode_order_cancelled(std::span<const std::uint8_t> bytes);

struct OrderReplaced {
  std::uint64_t original_order_reference_number;
  std::uint64_t new_order_reference_number;
  Quantity quantity;
  Ticks price;
};

OrderReplaced decode_order_replaced(std::span<const std::uint8_t> bytes);

using DecodedMessage =
    std::variant<StockDirectory, OrderDelete, AddOrder, OrderExecuted, OrderExecutedWithPrice,
                 OrderCancelled, OrderReplaced, UnknownMessage>;

DecodedMessage decode_message(std::span<const std::uint8_t> bytes);

}  // namespace engine
