#include "engine/stock_locate.hpp"

#include <optional>
#include <variant>

#include "engine/itch_framing.hpp"
#include "engine/itch_message.hpp"

namespace engine {

std::optional<std::uint16_t> resolve_stock_locate(std::istream& stream, std::string_view symbol) {
  bool has_seen_the_first_stock_locate_message = false;
  bool has_checked_all_stock_locate_messages = false;

  while (has_seen_the_first_stock_locate_message == false ||
         has_checked_all_stock_locate_messages == false) {
    auto bytes = read_next_message(stream);

    if (!bytes) {
      return std::nullopt;
    }

    auto decoded_message = decode_message(*bytes);

    if (std::holds_alternative<StockDirectory>(decoded_message)) {
      has_seen_the_first_stock_locate_message = true;
      StockDirectory stock_directory = std::get<StockDirectory>(decoded_message);
      if (stock_directory.stock == symbol) {
        return stock_directory.stock_locate;
      }
    } else if (has_seen_the_first_stock_locate_message) {
      has_checked_all_stock_locate_messages = true;
    }
  }

  return std::nullopt;
};

}  // namespace engine