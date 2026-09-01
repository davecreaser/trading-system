#include "engine/itch_message.hpp"
#include "engine/byte_reader.hpp"
#include <span>

namespace engine {

StockDirectory decode_stock_directory(std::span<const std::uint8_t> bytes) {
    StockDirectory stock_directory;

    stock_directory.stock_locate = read_be<std::uint16_t>(bytes, 1);
    
    auto stock_bytes = bytes.subspan(11, 8);
    stock_directory.stock = std::string(reinterpret_cast<const char*>(stock_bytes.data()), stock_bytes.size());

    std::size_t last = stock_directory.stock.find_last_not_of(' ');
    stock_directory.stock.erase(last + 1);

    return stock_directory;
};

OrderDelete decode_order_delete(std::span<const std::uint8_t> bytes) {
    OrderDelete order_delete;

    order_delete.order_reference_number = read_be<std::uint64_t>(bytes, 11);

    return order_delete;
};

DecodedMessage decode_message(std::span<const std::uint8_t> bytes) {
    auto message_type = bytes[0];

    switch (message_type) {
        case 'R':
            return decode_stock_directory(bytes);
        case 'D':
            return decode_order_delete(bytes);
        default:
            return UnknownMessage{message_type, std::vector<std::uint8_t>(bytes.begin(), bytes.end())};
    }
};

} // namespace engine
