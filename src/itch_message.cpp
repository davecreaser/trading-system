#include "engine/itch_message.hpp"

#include <span>

#include "engine/byte_reader.hpp"

namespace engine {

StockDirectory decode_stock_directory(std::span<const std::uint8_t> bytes) {
  StockDirectory stock_directory;

  stock_directory.stock_locate = read_be<std::uint16_t>(bytes, 1);

  auto stock_bytes = bytes.subspan(11, 8);
  stock_directory.stock =
      std::string(reinterpret_cast<const char*>(stock_bytes.data()), stock_bytes.size());

  std::size_t last = stock_directory.stock.find_last_not_of(' ');
  stock_directory.stock.erase(last + 1);

  return stock_directory;
};

OrderDelete decode_order_delete(std::span<const std::uint8_t> bytes) {
  OrderDelete order_delete;

  order_delete.order_reference_number = read_be<std::uint64_t>(bytes, 11);

  return order_delete;
};

AddOrder decode_add_order(std::span<const std::uint8_t> bytes) {
  AddOrder add_order;

  add_order.stock_locate = read_be<std::uint16_t>(bytes, 1);
  add_order.order_reference_number = read_be<std::uint64_t>(bytes, 11);
  add_order.side = bytes[19];
  add_order.quantity = read_be<std::uint32_t>(bytes, 20);
  add_order.price = read_be<std::uint32_t>(bytes, 32);

  return add_order;
};

OrderExecuted decode_order_executed(std::span<const std::uint8_t> bytes) {
  OrderExecuted order_executed;

  order_executed.order_reference_number = read_be<std::uint64_t>(bytes, 11);
  order_executed.quantity = read_be<std::uint32_t>(bytes, 19);

  return order_executed;
}

OrderExecutedWithPrice decode_order_executed_with_price(std::span<const std::uint8_t> bytes) {
  OrderExecutedWithPrice order_executed_with_price;

  order_executed_with_price.order_reference_number = read_be<std::uint64_t>(bytes, 11);
  order_executed_with_price.quantity = read_be<std::uint32_t>(bytes, 19);
  order_executed_with_price.price = read_be<std::uint32_t>(bytes, 32);

  return order_executed_with_price;
}

OrderCancelled decode_order_cancelled(std::span<const std::uint8_t> bytes) {
  OrderCancelled order_cancelled;

  order_cancelled.order_reference_number = read_be<std::uint64_t>(bytes, 11);
  order_cancelled.quantity = read_be<std::uint32_t>(bytes, 19);

  return order_cancelled;
}

OrderReplaced decode_order_replaced(std::span<const std::uint8_t> bytes) {
  OrderReplaced order_replaced;

  order_replaced.original_order_reference_number = read_be<std::uint64_t>(bytes, 11);
  order_replaced.new_order_reference_number = read_be<std::uint64_t>(bytes, 19);
  order_replaced.quantity = read_be<std::uint32_t>(bytes, 27);
  order_replaced.price = read_be<std::uint32_t>(bytes, 31);

  return order_replaced;
}

DecodedMessage decode_message(std::span<const std::uint8_t> bytes) {
  auto message_type = bytes[0];

  switch (message_type) {
    case 'R':
      return decode_stock_directory(bytes);
    case 'D':
      return decode_order_delete(bytes);
    case 'A':
      return decode_add_order(bytes);
    case 'F':
      return decode_add_order(bytes);
    case 'E':
      return decode_order_executed(bytes);
    case 'C':
      return decode_order_executed_with_price(bytes);
    case 'X':
      return decode_order_cancelled(bytes);
    case 'U':
      return decode_order_replaced(bytes);
    default:
      return UnknownMessage{message_type, std::vector<std::uint8_t>(bytes.begin(), bytes.end())};
  }
};

}  // namespace engine
