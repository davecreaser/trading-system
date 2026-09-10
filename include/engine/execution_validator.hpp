#pragma once

#include <cstddef>
#include <optional>
#include <unordered_map>
#include <vector>

#include "engine/itch_message.hpp"
#include "engine/order.hpp"
#include "engine/order_book_driver.hpp"

namespace engine {

struct Mismatch {
  OrderId order_id;
  Quantity book_total;
  Quantity itch_total;
};

struct ValidationSummary {
  std::size_t orders_checked;
  std::vector<Mismatch> mismatches;
};

class ExecutionValidator {
public:
  explicit ExecutionValidator(const OrderBookDriver& driver);

  void record(const DecodedMessage& message, const std::optional<AddResult>& result);

  ValidationSummary summary() const;

private:
  const OrderBookDriver& driver_;
  std::unordered_map<OrderId, Quantity> book_totals_;
  std::unordered_map<OrderId, Quantity> itch_totals_;
};

}  // namespace engine
