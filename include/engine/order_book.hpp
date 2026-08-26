#pragma once

#include <unordered_map>
#include <optional>
#include "engine/order.hpp"

namespace engine {

class OrderBook {
    public:
        AddResult add(Side side, Ticks price, Quantity quantity);
        bool cancel(OrderId id);
        std::optional<Ticks> best_bid() const;
        std::optional<Ticks> best_ask() const;

        const Bids& bids() const;
        const Asks& asks() const;

    private:
        void remove_resting_order(std::list<Order>::iterator iterator);
        struct OrderLocation { Side side; Ticks price; std::list<Order>::iterator iterator; };

        std::unordered_map<OrderId, OrderLocation> order_index_;
        Bids bids_;
        Asks asks_;
        OrderId next_order_id_ = 1;
};

} // namespace engine