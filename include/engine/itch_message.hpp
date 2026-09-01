#pragma once

#include <cstdint>
#include <span>
#include <string>
#include <variant>

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

using DecodedMessage = std::variant<StockDirectory, OrderDelete, UnknownMessage>;

DecodedMessage decode_message(std::span<const std::uint8_t> bytes);

} // namespace engine
