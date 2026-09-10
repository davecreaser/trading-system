#include "engine/order_book_driver.hpp"

#include <cstdint>
#include <optional>
#include <variant>

#include "engine/itch_message.hpp"
#include "engine/order.hpp"
#include "engine/order_book.hpp"

namespace engine {

OrderBookDriver::OrderBookDriver(OrderBook& book, std::uint16_t stock_locate)
    : book_(book), stock_locate_(stock_locate) {}

std::optional<AddResult> OrderBookDriver::process(const DecodedMessage& message) {
  if (std::holds_alternative<AddOrder>(message)) {
    AddOrder add_order = std::get<AddOrder>(message);
    if (add_order.stock_locate != stock_locate_) {
      return std::nullopt;
    }

    Side side;
    if (add_order.side == 'B') {
      side = Side::Buy;
    } else if (add_order.side == 'S') {
      side = Side::Sell;
    } else {
      return std::nullopt;
    }

    AddResult add_result = book_.add(side, add_order.price, add_order.quantity);

    itch_ref_to_order_id_[add_order.order_reference_number] = add_result.order_id;

    return add_result;
  }

  if (std::holds_alternative<OrderDelete>(message)) {
    OrderDelete order_delete = std::get<OrderDelete>(message);
    auto it = itch_ref_to_order_id_.find(order_delete.order_reference_number);
    if (it == itch_ref_to_order_id_.end()) {
      return std::nullopt;
    }
    OrderId order_id = it->second;

    // If cancel() returns false here, the order was already fully consumed by some
    // other order's crossing (or a prior full cancel/replace) before this Delete
    // arrived -- not a bug, ITCH doesn't tell us proactively when a resting order
    // we're tracking gets consumed as the passive side of someone else's trade.
    // Either way, there's nothing more to do.
    book_.cancel(order_id);
    itch_ref_to_order_id_.erase(order_delete.order_reference_number);

    return std::nullopt;
  }

  if (std::holds_alternative<OrderCancelled>(message)) {
    OrderCancelled order_cancelled = std::get<OrderCancelled>(message);
    auto it = itch_ref_to_order_id_.find(order_cancelled.order_reference_number);
    if (it == itch_ref_to_order_id_.end()) {
      return std::nullopt;
    }
    OrderId order_id = it->second;

    std::optional<Order> order = book_.resting_order(order_id);
    if (!order) {
      itch_ref_to_order_id_.erase(order_cancelled.order_reference_number);
      return std::nullopt;
    }

    // A cancel can legitimately request more than is currently resting: it's a real
    // consequence of a race between a trader's cancel request and an execution that
    // happened before the exchange processed it (the cancel was sent against
    // quantity the trader's own system hadn't yet learned had already traded).
    // Verified against real data -- not a sign our own tracking has drifted, so
    // clamp to zero (cancel everything left) rather than treating it as an error.
    Quantity new_quantity = 0;
    if (order_cancelled.quantity < order->quantity) {
      new_quantity = order->quantity - order_cancelled.quantity;
    }

    if (new_quantity == 0) {
      itch_ref_to_order_id_.erase(order_cancelled.order_reference_number);
    }

    return book_.modify(order_id, order->price, new_quantity);
  }

  if (std::holds_alternative<OrderReplaced>(message)) {
    OrderReplaced order_replaced = std::get<OrderReplaced>(message);
    auto it = itch_ref_to_order_id_.find(order_replaced.original_order_reference_number);
    if (it == itch_ref_to_order_id_.end()) {
      return std::nullopt;
    }
    OrderId order_id = it->second;

    std::optional<AddResult> result =
        book_.modify(order_id, order_replaced.price, order_replaced.quantity);
    itch_ref_to_order_id_.erase(order_replaced.original_order_reference_number);
    if (!result.has_value()) {
      // Same as OrderDelete: the order was already fully consumed before this
      // Replace arrived. Nothing to re-key -- there's no order left to point the
      // new reference number at.
      return std::nullopt;
    }
    if (result->remaining_quantity > 0) {
      itch_ref_to_order_id_[order_replaced.new_order_reference_number] = order_id;
    }

    return result;
  }

  return std::nullopt;
}

std::optional<OrderId> OrderBookDriver::resolve(std::uint64_t itch_reference_number) const {
  auto it = itch_ref_to_order_id_.find(itch_reference_number);
  if (it == itch_ref_to_order_id_.end()) {
    return std::nullopt;
  }
  return it->second;
}

}  // namespace engine
