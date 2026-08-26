#pragma once

#include <cstdint>
#include <map>
#include <list>
#include <functional>
#include <vector>

namespace engine {

using OrderId = std::uint64_t; // The ID of an order in the order book
using Ticks = std::int64_t; // The price of an order in ticks
using Quantity = std::uint64_t; // The quantity of a bid or ask

enum class Side { Buy, Sell }; // Which side can an order be on

struct Order {  // The whole order object
    OrderId id;
    Side side;
    Ticks price;
    Quantity quantity;
};

using Bids = std::map<Ticks, std::list<Order>, std::greater<Ticks>>; // Map of list of bids for each price point, descending
using Asks = std::map<Ticks, std::list<Order>>; // Map of list of asks for each price point, ascending

struct Fill { // Details of an order fill
    OrderId resting_order_id;
    OrderId incoming_order_id;
    Ticks price;
    Quantity quantity;
};

struct AddResult { // Return value for Add()
    OrderId order_id;
    std::vector<Fill> fills;
    Quantity remaining_quantity;
};

} // namespace engine
