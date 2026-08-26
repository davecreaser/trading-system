#include "engine/order_book.hpp"
#include "engine/order.hpp"

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

AddResult OrderBook::add(Side side, Ticks price, Quantity quantity) {
    Order order;

    order.id = next_order_id_++;
    order.price = price;
    order.quantity = quantity;
    order.side = side;

    // If the order crosses
    std::list<Order>::iterator resting_order;
    Fill fill;
    bool crossed = false;
    if (side == Side::Buy) {
        if (asks_.size() > 0 && asks_.begin()->first <= price) {
            resting_order = asks_.begin()->second.begin();
            crossed = true;
        }
    } else {
        if (bids_.size() > 0 && bids_.begin()->first >= price) {
            resting_order = bids_.begin()->second.begin();
            crossed = true;
        }
    }

    if (crossed == true) {
        OrderLocation location = order_index_.find(resting_order->id)->second;
        Quantity matched_quantity = quantity;
        fill.incoming_order_id = order.id;
        fill.resting_order_id= resting_order->id;
        fill.price = resting_order->price;

        // Case A: Order quantities are equal
        if (resting_order->quantity == quantity) {
            order.quantity = 0;
            remove_resting_order(location.iterator);
        }

        // Case B: Resting order fully consumed, incoming has leftover
        else if (resting_order->quantity < quantity) {
            matched_quantity = resting_order->quantity;
            order.quantity = quantity - resting_order->quantity;
            remove_resting_order(location.iterator);
        }

        // Case C: Incoming order fully consumed, resting has leftover
        else if (resting_order->quantity > quantity) {
            order.quantity = 0;
            resting_order->quantity = resting_order->quantity - quantity;
        }

        fill.quantity = matched_quantity;
    }

    // If the order will rest in the book
    if (order.quantity > 0) {
        std::list<Order>::iterator iterator;

        if (side == Side::Buy) {
            iterator = bids_[price].insert(bids_[price].end(), order);
        } else {
            iterator = asks_[price].insert(asks_[price].end(), order);
        }

        order_index_[order.id] = OrderLocation{order.side, order.price, iterator};
    }

    AddResult result;

    result.order_id = order.id;
    result.remaining_quantity = order.quantity;

    if (crossed) {
        result.fills.push_back(fill);
    }

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