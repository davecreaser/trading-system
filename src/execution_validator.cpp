#include "engine/execution_validator.hpp"

#include <cstddef>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

#include "engine/itch_message.hpp"
#include "engine/order.hpp"
#include "engine/order_book_driver.hpp"

namespace engine {

ExecutionValidator::ExecutionValidator(const OrderBookDriver& driver) : driver_(driver) {}

void ExecutionValidator::record(const DecodedMessage& message,
                                const std::optional<AddResult>& result) {
  if (result.has_value() && !result->fills.empty()) {
    for (Fill fill : result->fills) {
      book_totals_[fill.resting_order_id] += fill.quantity;
    }
  }

  std::optional<OrderId> order_id = std::nullopt;
  Quantity quantity = 0;

  if (std::holds_alternative<OrderExecuted>(message)) {
    OrderExecuted order_executed = std::get<OrderExecuted>(message);
    order_id = driver_.resolve(order_executed.order_reference_number);
    quantity = order_executed.quantity;
  } else if (std::holds_alternative<OrderExecutedWithPrice>(message)) {
    OrderExecutedWithPrice order_executed_with_price = std::get<OrderExecutedWithPrice>(message);
    order_id = driver_.resolve(order_executed_with_price.order_reference_number);
    quantity = order_executed_with_price.quantity;
  }

  if (order_id == std::nullopt) {
    return;
  }

  itch_totals_[*order_id] += quantity;
}

ValidationSummary ExecutionValidator::summary() const {
  ValidationSummary summary{0, std::vector<Mismatch>{}};

  std::unordered_set<OrderId> all_order_ids;
  for (const auto& [order_id, quantity] : book_totals_) {
    all_order_ids.insert(order_id);
  }
  for (const auto& [order_id, quantity] : itch_totals_) {
    all_order_ids.insert(order_id);
  }

  for (OrderId order_id : all_order_ids) {
    Quantity book_total = 0;
    auto it_book = book_totals_.find(order_id);
    if (it_book == book_totals_.end()) {
      book_total = 0;
    } else {
      book_total = it_book->second;
    }

    Quantity itch_total = 0;
    auto it_itch = itch_totals_.find(order_id);
    if (it_itch == itch_totals_.end()) {
      itch_total = 0;
    } else {
      itch_total = it_itch->second;
    }

    if (book_total != itch_total) {
      summary.mismatches.push_back(Mismatch{order_id, book_total, itch_total});
    }

    summary.orders_checked += 1;
  }

  return summary;
}

}  // namespace engine
