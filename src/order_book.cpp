#include "engine/order_book.hpp"
#include "engine/order.hpp"
#include <optional>

namespace engine {

void OrderBook::remove_resting_order(std::list<Order>::iterator iterator) {
    Side side = iterator->side;
    OrderId id = iterator-> id;
    Ticks price = iterator-> price;

    if (side == Side::Buy) {
        bids_[price].erase(iterator);

        if (bids_[price].empty()) {
            bids_.erase(price);
        }
    } else {
        asks_[price].erase(iterator);

        if (asks_[price].empty()) {
            asks_.erase(price);
        }
    }

    order_index_.erase(id);
};

AddResult OrderBook::resolve_order(Order order) {
    // If the order crosses
    std::list<Order>::iterator resting_order;
    std::vector<Fill> fills;
    bool crossed = true;

    while (order.quantity > 0 && crossed == true) {
        if (order.side == Side::Buy) {
            if (asks_.size() > 0 && asks_.begin()->first <= order.price) {
                resting_order = asks_.begin()->second.begin();
                crossed = true;
            } else {
                crossed = false;
            }
        } else {
            if (bids_.size() > 0 && bids_.begin()->first >= order.price) {
                resting_order = bids_.begin()->second.begin();
                crossed = true;
            } else {
                crossed = false;
            }
        }

        if (crossed == true) {
            OrderLocation location = order_index_.find(resting_order->id)->second;
            Quantity matched_quantity = order.quantity;
            Fill fill;
            fill.incoming_order_id = order.id;
            fill.resting_order_id= resting_order->id;
            fill.price = resting_order->price;

            // Case A: Order quantities are equal
            if (resting_order->quantity == order.quantity) {
                order.quantity = 0;
                remove_resting_order(location.iterator);
            }

            // Case B: Resting order fully consumed, incoming has leftover
            else if (resting_order->quantity < order.quantity) {
                matched_quantity = resting_order->quantity;
                order.quantity = order.quantity - resting_order->quantity;
                remove_resting_order(location.iterator);
            }

            // Case C: Incoming order fully consumed, resting has leftover
            else if (resting_order->quantity > order.quantity) {
                resting_order->quantity = resting_order->quantity - order.quantity;
                order.quantity = 0;
            }

            fill.quantity = matched_quantity;
            fills.push_back(fill);
        }
    }

    // If the order will rest in the book
    if (order.quantity > 0) {
        std::list<Order>::iterator iterator;

        if (order.side == Side::Buy) {
            iterator = bids_[order.price].insert(bids_[order.price].end(), order);
        } else {
            iterator = asks_[order.price].insert(asks_[order.price].end(), order);
        }

        order_index_[order.id] = OrderLocation{order.side, order.price, iterator};
    }

    AddResult result;

    result.order_id = order.id;
    result.remaining_quantity = order.quantity;
    result.fills = fills;

    return result;
};

AddResult OrderBook::add(Side side, Ticks price, Quantity quantity) {
    Order order;

    order.id = next_order_id_++;
    order.price = price;
    order.quantity = quantity;
    order.side = side;

    AddResult result = resolve_order(order);

    return result;
}

bool OrderBook::cancel(OrderId id) {
    auto it = order_index_.find(id);
    if (it == order_index_.end()) {
        return false;
    }

    OrderLocation location = it->second;

    remove_resting_order(location.iterator);

    return true;
}

std::optional<AddResult> OrderBook::modify(OrderId id, Ticks new_price, Quantity new_quantity) {
    auto it = order_index_.find(id);
    if (it == order_index_.end()) {
        return std::nullopt;
    }

    std::list<Order>::iterator order = it->second.iterator;
    OrderId current_order_id = order->id;
    Ticks current_price = order->price;
    Quantity current_quantity = order->quantity;
    Side current_side = order->side;
    AddResult result;

    if (new_price == current_price && new_quantity == current_quantity) {
        result.remaining_quantity = current_quantity;
        result.order_id = current_order_id;
        return result;
    }

    if (new_price == current_price && new_quantity <= current_quantity) {
        order->quantity = new_quantity;
        result.order_id = order->id;
        result.remaining_quantity = new_quantity;
    } else {
        remove_resting_order(order);
        Order new_order;
        new_order.id = current_order_id;
        new_order.price = new_price;
        new_order.quantity = new_quantity;
        new_order.side = current_side;
        result = resolve_order(new_order);
    }

    return result;
};

std::optional<Ticks> OrderBook::best_bid() const {
    if (bids_.size() == 0) {
        return std::nullopt;
    }

    return bids_.begin()->first;
}

std::optional<Ticks> OrderBook::best_ask() const {
    if (asks_.size() == 0) {
        return std::nullopt;
    }

    return asks_.begin()->first;
}

const Bids& OrderBook::bids() const {
    return bids_;
}

const Asks& OrderBook::asks() const {
    return asks_;
}


}  // namespace engine