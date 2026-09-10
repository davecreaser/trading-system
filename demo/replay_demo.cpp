#include <cstddef>
#include <cstdint>
#include <exception>
#include <fstream>
#include <iostream>
#include <optional>

#include "engine/execution_validator.hpp"
#include "engine/itch_framing.hpp"
#include "engine/itch_message.hpp"
#include "engine/order.hpp"
#include "engine/order_book.hpp"
#include "engine/order_book_driver.hpp"
#include "engine/stock_locate.hpp"

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "usage: engine_demo <path to decompressed ITCH file>\n";
    return 1;
  }

  std::ifstream file(argv[1], std::ios::binary);
  if (!file) {
    std::cerr << "could not open file: " << argv[1] << "\n";
    return 1;
  }

  try {
    std::optional<std::uint16_t> locate = engine::resolve_stock_locate(file, "AAPL");
    if (!locate) {
      std::cerr << "AAPL not found in this file\n";
      return 1;
    }

    engine::OrderBook book{};
    engine::OrderBookDriver driver{book, *locate};
    engine::ExecutionValidator validator{driver};

    std::size_t total_messages_processed = 0;
    std::size_t fills_produced = 0;
    std::size_t total_executed_volume = 0;
    std::optional<engine::Ticks> best_bid{};
    std::optional<engine::Ticks> best_ask{};

    while (auto bytes = engine::read_next_message(file)) {
      auto message = engine::decode_message(*bytes);
      auto result = driver.process(message);
      validator.record(message, result);

      total_messages_processed++;

      if (total_messages_processed % 1000000 == 0) {
        std::cerr << "... " << total_messages_processed << " messages processed so far\n";
      }

      if (result.has_value() && !result->fills.empty()) {
        fills_produced += result->fills.size();

        for (engine::Fill fill : result->fills) {
          total_executed_volume += fill.quantity;
        }
      }

      if (book.best_bid() != best_bid || book.best_ask() != best_ask) {
        std::cout << "#" << total_messages_processed << ": bid=";
        if (book.best_bid()) {
          std::cout << *book.best_bid();
        } else {
          std::cout << "none";
        }
        std::cout << " ask=";
        if (book.best_ask()) {
          std::cout << *book.best_ask();
        } else {
          std::cout << "none";
        }
        std::cout << "\n";

        best_bid = book.best_bid();
        best_ask = book.best_ask();
      }
    }

    engine::ValidationSummary summary = validator.summary();

    std::cout << "\n--- summary ---\n";
    std::cout << "messages processed: " << total_messages_processed << "\n";
    std::cout << "AAPL stock locate: " << *locate << "\n";
    std::cout << "fills produced: " << fills_produced << "\n";
    std::cout << "total executed volume: " << total_executed_volume << "\n";
    std::cout << "orders checked: " << summary.orders_checked << "\n";
    std::cout << "mismatches: " << summary.mismatches.size() << "\n";
    for (const engine::Mismatch& mismatch : summary.mismatches) {
      std::cout << "  order_id=" << mismatch.order_id << " book_total=" << mismatch.book_total
                << " itch_total=" << mismatch.itch_total << "\n";
    }
  } catch (const std::exception& e) {
    std::cerr << "error: " << e.what() << "\n";
    return 1;
  }

  return 0;
}
